#!/usr/bin/env bash
# Misst auf jedem GPU-Typ des Clusters genau einmal.
# Auf dem LOGIN-Knoten im Ordner sycl-matmul ausführen:
#
#   bash dachs/alle_gpus.sh                 # alle GPU-Typen
#   TYPEN="l40s h200" bash dachs/alle_gpus.sh   # nur bestimmte Typen
#   DRY=1 bash dachs/alle_gpus.sh           # nur anzeigen, nichts bauen/abschicken
#
# Ablauf:
#   1. GPU-Typen aus "scontrol show partition" lesen (gres/gpu:<typ>);
#      je Typ wird die erste Partition genommen, die ihn anbietet
#   2. Je Hersteller einmal bauen, in eigene Ordner (build_nvidia/, build_amd/),
#      damit sich die Builds nicht überschreiben. Ein NVIDIA-Build reicht für
#      alle NVIDIA-Typen, da PTX auf jedem Gerät just in time übersetzt wird.
#   3. Je Typ einen Job abschicken: sbatch -p <partition> --gres=gpu:<typ>:1
#
# Ergebnis: ergebnisse_dachs/<typ>_<jobid>/  (Status: squeue --me)

set -uo pipefail
cd "$(dirname "$0")/.."
ROOT="$PWD"

info() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33mWarnung:\033[0m %s\n' "$*" >&2; }
fail() { printf '\033[1;31mFehler:\033[0m %s\n' "$*" >&2; exit 1; }

[ -f "$HOME/sycl/env.sh" ] || fail "~/sycl/env.sh fehlt (erst bash dachs/install_dpcpp.sh)"
# shellcheck disable=SC1091
source "$HOME/sycl/env.sh"
: "${DPCPP_HOME:?DPCPP_HOME nicht gesetzt}"

# ---------------------------------------------------------------------------
# 1. GPU-Typen und Partitionen finden
# ---------------------------------------------------------------------------
declare -A PARTITION    # typ -> partition
REIHENFOLGE=()
while read -r zeile; do
  part=$(grep -o 'PartitionName=[^ ]*' <<<"$zeile" | cut -d= -f2)
  for typ in $(grep -o 'gres/gpu:[^=,]*' <<<"$zeile" | cut -d: -f2); do
    if [ -z "${PARTITION[$typ]:-}" ]; then
      PARTITION[$typ]="$part"
      REIHENFOLGE+=("$typ")
    fi
  done
done < <(scontrol show partition -o)

[ ${#REIHENFOLGE[@]} -gt 0 ] || fail "Keine GPU-Partitionen gefunden (scontrol show partition)"

if [ -n "${TYPEN:-}" ]; then
  REIHENFOLGE=($TYPEN)
fi

hersteller() {
  case "$1" in
    mi*|gfx*|radeon*|instinct*) echo amd ;;
    *) echo nvidia ;;
  esac
}

info "Gefundene GPU-Typen:"
for typ in "${REIHENFOLGE[@]}"; do
  [ -n "${PARTITION[$typ]:-}" ] || fail "Typ '$typ' wird von keiner Partition angeboten"
  printf '    %-8s  Partition %-12s (%s)\n' "$typ" "${PARTITION[$typ]}" "$(hersteller "$typ")"
done

# ---------------------------------------------------------------------------
# 2. Voraussetzungen je Hersteller prüfen und bauen
# ---------------------------------------------------------------------------
amd_bereit() {
  if ! ls "$DPCPP_HOME/lib" | grep -q '^libur_adapter_hip\.so'; then
    warn "AMD übersprungen: $DPCPP_HOME/lib enthält keinen HIP-Adapter (libur_adapter_hip.so)."
    warn "  Das DPC++-Release kann AMD-GPUs damit nicht ansprechen."
    return 1
  fi
  if [ -z "${ROCM_PATH:-}" ] && [ ! -d /opt/rocm ]; then
    warn "AMD übersprungen: kein ROCm gefunden (ROCM_PATH leer, /opt/rocm fehlt)."
    return 1
  fi
  return 0
}

declare -A GEBAUT     # hersteller -> build-ordner (leer = nicht verfügbar)
for typ in "${REIHENFOLGE[@]}"; do
  h=$(hersteller "$typ")
  [ -n "${GEBAUT[$h]+x}" ] && continue
  if [ "$h" = amd ] && ! amd_bereit; then
    GEBAUT[$h]=""
    continue
  fi
  if [ -n "${DRY:-}" ]; then
    GEBAUT[$h]="$ROOT/build_$h"
    continue
  fi
  info "Baue für $h nach build_$h/"
  if GPU="$h" BUILD_DIR="$ROOT/build_$h" bash dachs/build.sh > "build_$h.log" 2>&1; then
    GEBAUT[$h]="$ROOT/build_$h"
  else
    warn "Bauen für $h fehlgeschlagen, siehe build_$h.log (letzte Zeilen):"
    tail -n 15 "build_$h.log" >&2
    GEBAUT[$h]=""
  fi
done

# ---------------------------------------------------------------------------
# 3. Je GPU-Typ einen Job abschicken
# ---------------------------------------------------------------------------
mkdir -p ergebnisse_dachs
ABGESCHICKT=()
for typ in "${REIHENFOLGE[@]}"; do
  h=$(hersteller "$typ")
  dir="${GEBAUT[$h]:-}"
  if [ -z "$dir" ]; then
    warn "$typ wird nicht gemessen (kein Build für $h)."
    continue
  fi
  cmd=(sbatch -p "${PARTITION[$typ]}" --gres="gpu:$typ:1" --job-name="sycl-$typ"
       --export="ALL,BUILD_DIR=$dir,GPU_TYP=$typ" dachs/messung.sbatch)
  if [ -n "${DRY:-}" ]; then
    echo "    ${cmd[*]}"
  else
    antwort=$("${cmd[@]}") || { warn "Abschicken für $typ fehlgeschlagen"; continue; }
    ABGESCHICKT+=("$typ: $antwort")
  fi
done

echo
if [ ${#ABGESCHICKT[@]} -gt 0 ]; then
  info "Abgeschickt:"
  printf '    %s\n' "${ABGESCHICKT[@]}"
  echo "Status: squeue --me    Ergebnisse: ergebnisse_dachs/<typ>_<jobid>/"
fi
