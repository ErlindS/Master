#!/bin/bash -l
# Alle drei Beispiele (Matmul, Stencil, Reduktion) mit AdaptiveCpp auf der AMD MI300A
# bauen und messen. Gebaut wird im Job, weil ROCm nur auf dem Knoten sicher vorhanden ist.
#
# Voraussetzung: bash gemeinsam/dachs/amd.sh install   (einmalig)
# Abschicken (im Ordner code):  bash gemeinsam/dachs/amd.sh messen
#   nur ein Beispiel:           BEISPIELE="Matmul" bash gemeinsam/dachs/amd.sh messen
#   CPU des APU mitmessen:      MIT_CPU=1 bash gemeinsam/dachs/amd.sh messen
#
# Ergebnisse (gleiches Format wie bei L40S/H100):
#   Matmul/ergebnisse/dachs/mi300a_<jobid>/{system.txt,stufen_gpu.csv,kacheln.csv,portabilitaet.txt}
#   Stencil/ergebnisse/dachs/mi300a_<jobid>/{system.txt,stencil.csv}
#   Reduktion/ergebnisse/dachs/mi300a_<jobid>/{system.txt,reduktion.csv}
#
#SBATCH --job-name=sycl-mi300a
#SBATCH --partition=gpu_mi300a
#SBATCH --gres=gpu:mi300a:1
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --time=02:00:00
#SBATCH --output=ergebnisse_dachs/sycl_mi300a_%j.out

set -uo pipefail
CODE="$(cd "${SLURM_SUBMIT_DIR:-.}" && pwd)"    # Ordner code
cd "$CODE"
[ -f "$HOME/sycl/env_amd.sh" ] || { echo "~/sycl/env_amd.sh fehlt – erst: bash gemeinsam/dachs/amd.sh install"; exit 1; }
# shellcheck disable=SC1091
source "$HOME/sycl/env_amd.sh"

BEISPIELE="${BEISPIELE:-Matmul Stencil Reduktion}"
TYP="${GPU_TYP:-mi300a}"
ID="${SLURM_JOB_ID:-0}"
BUILD="build_amd"
export ACPP_VISIBILITY_MASK="hip"        # entspricht ONEAPI_DEVICE_SELECTOR=hip:gpu
GERAETE="hip"; [ -n "${MIT_CPU:-}" ] && GERAETE="hip omp"

# Eigener, leerer Kernel-Cache pro Job: sonst nimmt AdaptiveCpp beim ersten Lauf
# die schon übersetzten Kernel aus ~/.acpp und erster_lauf_ms enthält keine JIT-Zeit.
export ACPP_APPDB_DIR="${TMPDIR:-/tmp}/acpp_appdb_$ID"
rm -rf "$ACPP_APPDB_DIR"; mkdir -p "$ACPP_APPDB_DIR"

systeminfo() {
  echo "== Knoten";   hostname
  echo "== CPU";      lscpu | grep -E 'Model name|^CPU\(s\)'
  echo "== GPU"
  rocm-smi --showproductname 2>/dev/null || rocminfo 2>/dev/null | grep -m3 -E 'Marketing Name|gfx'
  echo "== ROCm";     cat "$ROCM_PATH/.info/version" 2>/dev/null || echo "$ROCM_PATH"
  echo "== Compiler"; acpp --acpp-version 2>&1 | grep -iE 'version|targets' | head -5
  echo "ACPP_TARGETS=$ACPP_TARGETS"
  echo "== SYCL-Geräte"; acpp-info -l 2>&1
}

for bsp in $BEISPIELE; do
  echo; echo "################ $bsp ################"
  cd "$CODE/$bsp" || continue
  OUT="$PWD/ergebnisse/dachs/${TYP}_$ID"
  mkdir -p "$OUT"
  systeminfo > "$OUT/system.txt" 2>&1
  rm -rf "$BUILD"

  case "$bsp" in
    Matmul)
      make BIN="$BUILD" COMPILER=acpp EXTRA="--acpp-targets=$ACPP_TARGETS" all kacheln \
        || { echo "Bauen von Matmul fehlgeschlagen"; continue; }
      BIN_DIR="$PWD/$BUILD" bash gemeinsam/run_benchmark.sh "$OUT/stufen_gpu.csv" \
        || echo "Warnung: run_benchmark.sh mit Fehler beendet (Teilergebnisse in stufen_gpu.csv)"
      BIN_DIR="$PWD/$BUILD" DEVICES="$GERAETE" SELECTOR_VAR=ACPP_VISIBILITY_MASK \
        bash gemeinsam/run_kacheln.sh "$OUT/kacheln.csv"
      python3 gemeinsam/portabilitaet.py "$OUT/kacheln.csv" > "$OUT/portabilitaet.txt" 2>&1
      ;;
    Stencil|Reduktion)
      make BIN="$BUILD" COMPILER=acpp EXTRA="--acpp-targets=$ACPP_TARGETS" sycl \
        || { echo "Bauen von $bsp fehlgeschlagen"; continue; }
      datei=$(echo "$bsp" | tr '[:upper:]' '[:lower:]').csv
      BIN_DIR="$BUILD" DEVICES="$GERAETE" SELECTOR_VAR=ACPP_VISIBILITY_MASK REPS=5 \
        bash gemeinsam/messen.sh "$OUT/$datei"
      ;;
  esac
  echo "Fertig: $OUT"
done

rm -rf "$ACPP_APPDB_DIR"
