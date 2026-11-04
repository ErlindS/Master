#!/bin/bash -l
# Messung auf einem DACHS-GPU-Knoten. Im Ordner sycl-matmul abschicken:
#
#   sbatch dachs/messung.sbatch              # L40S (gpu1, Standard)
#   sbatch -p gpu8 dachs/messung.sbatch      # H100
#
# Status:     squeue --me            (Spalte NODELIST(REASON) zeigt, worauf er wartet)
# Ergebnis:   ergebnisse_dachs/<partition>_<jobid>/
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
cd "${SLURM_SUBMIT_DIR:-.}"
source "$HOME/sycl/env.sh"

OUT="ergebnisse_dachs/${SLURM_JOB_PARTITION:-lokal}_${SLURM_JOB_ID:-0}"
mkdir -p "$OUT"

# Systeminfo für die Arbeit (Gerätenamen, Treiber, Compiler)
{
  echo "== Knoten";   hostname
  echo "== CPU";      lscpu | grep -E 'Model name|^CPU\(s\)'
  echo "== GPU";      nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv
  echo "== Compiler"; "$DPCPP_HOME/bin/clang++" --version | head -1
  echo "== SYCL-Geräte"; sycl-ls
} > "$OUT/system.txt" 2>&1
cat "$OUT/system.txt"

# Stufen 0–3 auf der GPU (wie gpu_nvidia.csv lokal)
ONEAPI_DEVICE_SELECTOR=cuda:gpu ./run_benchmark.sh "$OUT/stufen_gpu.csv"

# Kachelexperiment; CPU nur, wenn sycl-ls ein OpenCL-CPU-Gerät meldet
DEVICES="cuda:gpu"
sycl-ls 2>/dev/null | grep -q 'opencl:cpu' && DEVICES="opencl:cpu cuda:gpu"
DEVICES="$DEVICES" ./run_kacheln.sh "$OUT/kacheln.csv"
python3 portabilitaet.py "$OUT/kacheln.csv" > "$OUT/portabilitaet.txt"

echo "Fertig: $OUT"
