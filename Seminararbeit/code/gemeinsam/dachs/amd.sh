#!/usr/bin/env bash
# AMD MI300A auf DACHS: prüfen, AdaptiveCpp installieren, messen.
# Auf dem LOGIN-Knoten im Ordner code ausführen:
#
#   bash gemeinsam/dachs/amd.sh check     # was gibt es auf dem Knoten? (ROCm, Module, < 1 min)
#   bash gemeinsam/dachs/amd.sh install   # AdaptiveCpp bauen (einmalig, ca. 15 min)
#   bash gemeinsam/dachs/amd.sh messen    # Matmul, Stencil, Reduktion bauen + messen
#
# Die Partition wird aus "scontrol show partition" gelesen (gres gpu:mi300a);
# erzwingen mit PARTITION=<name>. Status: squeue --me
# Protokolle: ergebnisse_dachs/<job>_<jobid>.out

set -euo pipefail
SELF="$(readlink -f "$0")"          # vor dem cd, sonst stimmt ein relativer Pfad nicht mehr
cd "$(dirname "$SELF")/../.."       # Ordner code

TYP="${GPU_TYP:-mi300a}"
if [ -z "${PARTITION:-}" ]; then
  PARTITION=$(scontrol show partition -o 2>/dev/null | grep -i "gpu:$TYP" | grep -o 'PartitionName=[^ ]*' | head -1 | cut -d= -f2)
fi
[ -n "$PARTITION" ] || { echo "Keine Partition mit gres gpu:$TYP gefunden. PARTITION=<name> setzen." >&2; exit 1; }

mkdir -p ergebnisse_dachs
abschicken() {   # abschicken <sbatch-datei> [weitere sbatch-Optionen]
  local datei="$1"; shift
  echo "Partition $PARTITION, gres gpu:$TYP:1"
  sbatch -p "$PARTITION" --gres="gpu:$TYP:1" --export=ALL "$@" "$datei"
  echo "Status: squeue --me    (REASON zeigt, worauf der Job wartet; MI300A gibt es nur einmal)"
}

case "${1:-}" in
  check)
    abschicken gemeinsam/dachs/amd_check.sbatch ;;
  install)
    tb="${ACPP_TARBALL:-$HOME/AdaptiveCpp-25.10.0.tar.gz}"
    if [ ! -f "$tb" ]; then
      echo "$tb fehlt. Lokal laden und kopieren:" >&2
      echo "  curl -fL -o AdaptiveCpp-25.10.0.tar.gz https://github.com/AdaptiveCpp/AdaptiveCpp/archive/refs/tags/v25.10.0.tar.gz" >&2
      echo "  scp AdaptiveCpp-25.10.0.tar.gz hk_seer1012@dachs-login.hs-esslingen.de:~" >&2
      exit 1
    fi
    mkdir -p "$HOME/sycl"
    abschicken gemeinsam/dachs/install_acpp.sbatch ;;
  messen)
    warte=()
    if [ ! -f "$HOME/sycl/env_amd.sh" ]; then
      # Installation noch in der Warteschlange? Dann danach starten.
      inst=$(squeue --me -h -n install-acpp -o %i 2>/dev/null | head -1)
      [ -n "$inst" ] || { echo "Erst: bash gemeinsam/dachs/amd.sh install" >&2; exit 1; }
      echo "Installation (Job $inst) läuft noch – Messung startet erst, wenn sie erfolgreich war."
      warte=(--dependency="afterok:$inst")
    fi
    for b in ${BEISPIELE:-Matmul Stencil Reduktion}; do mkdir -p "$b/ergebnisse/dachs"; done
    abschicken gemeinsam/dachs/amd_messung.sbatch "${warte[@]}" ;;
  *)
    sed -n '2,12p' "$SELF"; exit 2 ;;
esac
