#!/bin/bash -l
# Messung auf einem DACHS-GPU-Knoten. Im Ordner sycl-matmul abschicken:
#
#   bash dachs/alle_gpus.sh                        # empfohlen: jeder GPU-Typ einmal
#   sbatch dachs/messung.sbatch                    # nur L40S (gpu1, Standard)
#   sbatch -p gpu8 dachs/messung.sbatch            # nur H100
#
# Das Gerät (cuda:gpu bzw. hip:gpu) wird per sycl-ls erkannt.
# Optional (setzt alle_gpus.sh): BUILD_DIR = Ordner mit den gebauten
# Programmen, GPU_TYP = Name für den Ergebnisordner.
# Status:     squeue --me            (Spalte NODELIST(REASON) zeigt, worauf er wartet)
# Ergebnis:   ergebnisse_dachs/<typ bzw. partition>_<jobid>/
#
#SBATCH --job-name=sycl-matmul
#SBATCH --partition=gpu1
#SBATCH --gres=gpu:1
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=32G
#SBATCH --time=01:00:00
#SBATCH --output=ergebnisse_dachs/slurm_%j.out

set -uo pipefail
ROOT="$(cd "${SLURM_SUBMIT_DIR:-.}" && pwd)"
cd "$ROOT"
source "$HOME/sycl/env.sh"

OUT="$ROOT/ergebnisse_dachs/${GPU_TYP:-${SLURM_JOB_PARTITION:-lokal}}_${SLURM_JOB_ID:-0}"
mkdir -p "$OUT"
[ -n "${BUILD_DIR:-}" ] && cd "$BUILD_DIR"

GERAETE="$(sycl-ls 2>&1)"
if echo "$GERAETE" | grep -q 'cuda:gpu'; then
  GPU_DEV="cuda:gpu"; GPU_ART="nvidia"
elif echo "$GERAETE" | grep -q 'hip:gpu'; then
  GPU_DEV="hip:gpu"; GPU_ART="amd"
else
  echo "Keine GPU gefunden. sycl-ls meldet:"; echo "$GERAETE"
  echo "Bei AMD: ROCm-Modul in ~/sycl/env.sh eintragen; libur_adapter_hip.so in \$DPCPP_HOME/lib?"
  exit 1
fi
if [ "$(cat .gebaut_fuer 2>/dev/null || echo nvidia)" != "$GPU_ART" ]; then
  echo "Programme wurden für $(cat .gebaut_fuer 2>/dev/null || echo nvidia) gebaut, der Knoten hat aber eine $GPU_ART-GPU."
  echo "Erst neu bauen: GPU=$GPU_ART bash dachs/build.sh"
  exit 1
fi

# Systeminfo für die Arbeit (Gerätenamen, Treiber, Compiler)
{
  echo "== Knoten";   hostname
  echo "== CPU";      lscpu | grep -E 'Model name|^CPU\(s\)'
  echo "== GPU"
  if [ "$GPU_ART" = nvidia ]; then
    nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv
  else
    rocm-smi --showproductname 2>/dev/null || rocminfo 2>/dev/null | grep -m3 -E 'Marketing Name|gfx'
  fi
  echo "== Compiler"; "$DPCPP_HOME/bin/clang++" --version | head -1
  echo "== SYCL-Geräte"; echo "$GERAETE"
} > "$OUT/system.txt" 2>&1
cat "$OUT/system.txt"

# Alle Stufen (0, 1, 2, 2b, 3, 4, 5) auf der GPU
ONEAPI_DEVICE_SELECTOR="$GPU_DEV" ./run_benchmark.sh "$OUT/stufen_gpu.csv"

# Kachelexperiment inkl. Stufe 5; CPU nur, wenn sycl-ls ein OpenCL-CPU-Gerät meldet
DEVICES="$GPU_DEV"
echo "$GERAETE" | grep -q 'opencl:cpu' && DEVICES="opencl:cpu $GPU_DEV"
DEVICES="$DEVICES" ./run_kacheln.sh "$OUT/kacheln.csv"
python3 portabilitaet.py "$OUT/kacheln.csv" > "$OUT/portabilitaet.txt"

echo "Fertig: $OUT"
