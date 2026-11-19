#!/bin/bash -l
# Smoke-Test der neuen Stufen-Varianten auf einer GPU: kleine Problemgrößen,
# zwei Wiederholungen, Blick auf die Spalte "korrekt".
#
# Als Job in der Warteschlange (empfohlen, kein Warten im Terminal).
# "dachs" wechselt selbst nach ~/code, deshalb ohne cd:
#   dachs sbatch -p gpu1 gemeinsam/dachs/smoke.sh     # L40S
#   dachs sbatch -p gpu8 gemeinsam/dachs/smoke.sh     # H100
#   -> Protokoll: ergebnisse_dachs/smoke_<jobid>.out
# Oder direkt (wartet, bis eine GPU frei ist):
#   srun -p gpu1 --gres=gpu:1 --time=00:20:00 bash gemeinsam/dachs/smoke.sh
#
# Einstellungen: N_STENCIL=1024, N_RED=4194304, REPS=2, SELECTOR=cuda:gpu
#
#SBATCH --job-name=smoke-stufen
#SBATCH --gres=gpu:1
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=32G
#SBATCH --time=00:20:00
#SBATCH --output=ergebnisse_dachs/smoke_%j.out

set -uo pipefail

# Ordner "code" finden. Achtung: sbatch kopiert dieses Skript in ein
# Spool-Verzeichnis des Rechenknotens, dort führt der Pfad des Skripts ins
# Leere. Darum zuerst SLURM_SUBMIT_DIR (das Verzeichnis, aus dem abgeschickt
# wurde), dann der Pfad des Skripts selbst.
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
WURZEL=""
for kandidat in "${SLURM_SUBMIT_DIR:-}" "${SLURM_SUBMIT_DIR:-}/.." \
                "$(dirname "$SELF")/../.." "$PWD" "$PWD/.." "$HOME/code"; do
  [ -n "$kandidat" ] || continue
  if [ -d "$kandidat/Stencil" ] && [ -d "$kandidat/Reduktion" ]; then
    WURZEL="$(cd "$kandidat" && pwd)"
    break
  fi
done
if [ -z "$WURZEL" ]; then
  echo "✗ Ordner 'code' nicht gefunden (Stencil/ und Reduktion/ fehlen)." >&2
  echo "  Abgeschickt aus: ${SLURM_SUBMIT_DIR:-<unbekannt>}, Skript: $SELF" >&2
  exit 1
fi
cd "$WURZEL"
echo "Arbeitsordner: $WURZEL"

if ! command -v module >/dev/null 2>&1; then
  for init in /etc/profile.d/modules.sh /etc/profile.d/z00_lmod.sh \
              /usr/share/lmod/lmod/init/bash; do
    # shellcheck disable=SC1090
    [ -r "$init" ] && . "$init" && break
  done
fi
ENV_SH="${SYCL_ENV:-$HOME/sycl/env.sh}"
# shellcheck disable=SC1090
[ -r "$ENV_SH" ] && . "$ENV_SH"

SELECTOR="${SELECTOR:-cuda:gpu}"
N_STENCIL="${N_STENCIL:-1024}"
N_RED="${N_RED:-4194304}"
REPS="${REPS:-2}"

echo "Knoten: $(hostname)"
nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null || echo "keine NVIDIA-GPU sichtbar"
echo "Selector: $SELECTOR, Stencil n=$N_STENCIL, Reduktion n=$N_RED, Wiederholungen=$REPS"
echo

ok=0; schlecht=0; fehlt=0

pruefe() {   # pruefe <ordner> <programm> <n> [parameter]
  local ordner="$1" prog="$2" n="$3" par="${4:-}" pfad="$1/bin/$2" zeile status
  if [ ! -x "$pfad" ]; then
    echo "FEHLT    $prog  (nicht gebaut – bash gemeinsam/dachs/bauen.sh)"
    fehlt=$((fehlt + 1)); return
  fi
  zeile=$(ONEAPI_DEVICE_SELECTOR="$SELECTOR" "$pfad" "$n" "$REPS" $par 2>&1)
  status=$(printf '%s\n' "$zeile" | grep -E '^[a-z].*;.*;' | tail -n 1 | awk -F';' '{print $NF}')
  case "$status" in
    ja)   echo "OK       $prog ${par:+(Parameter $par)}"
          printf '%s\n' "$zeile" | tail -n 1 | sed 's/^/         /'
          ok=$((ok + 1)) ;;
    NEIN) echo "FALSCH   $prog ${par:+(Parameter $par)} – Ergebnis stimmt nicht"
          printf '%s\n' "$zeile" | sed 's/^/         /'
          schlecht=$((schlecht + 1)) ;;
    *)    echo "ABBRUCH  $prog ${par:+(Parameter $par)} – keine Ergebniszeile:"
          printf '%s\n' "$zeile" | sed 's/^/         /'
          schlecht=$((schlecht + 1)) ;;
  esac
}

echo "--- Stencil (neue Stufen-Gegenstücke) ---"
for p in stencil4_gemeinsam stencil5_buffer stencil5_buffer_wieder stencil7_geraet \
         stencil7_geraet_flach; do
  pruefe Stencil "$p" "$N_STENCIL"
done
pruefe Stencil stencil6_gruppe "$N_STENCIL" 16

echo
echo "--- Stencil (bestehende Varianten als Gegenprobe) ---"
pruefe Stencil stencil1_2d "$N_STENCIL"

echo
echo "--- Reduktion (neue Stufen-Gegenstücke) ---"
for p in reduktion5_gemeinsam reduktion6_buffer reduktion6_buffer_wieder reduktion7_geraet; do
  pruefe Reduktion "$p" "$N_RED"
done

echo
echo "--- Reduktion (bestehende Variante als Gegenprobe) ---"
pruefe Reduktion reduktion1_atomar "$N_RED"

echo
echo "=== Ergebnis: $ok korrekt, $schlecht fehlerhaft, $fehlt nicht gebaut ==="
[ "$schlecht" -eq 0 ] && [ "$fehlt" -eq 0 ]
