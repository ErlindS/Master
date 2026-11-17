#!/bin/bash -l
# DACHS-Job für die Zusatzmessungen:
#   1. Matrixmultiplikation in nativem CUDA (../sycl-matmul/cuda)
#   2. Stencil und Reduktion in SYCL und CUDA (dieser Ordner)
#
# Vorher auf dem Login-Knoten bauen (siehe README, Abschnitt DACHS), dann im
# Ordner weitere-probleme abschicken:
#   sbatch -p gpu1 dachs.sbatch     # L40S
#   sbatch -p gpu8 dachs.sbatch     # H100
#
# Ergebnis: ergebnisse_dachs/<partition>_<jobid>/
#
#SBATCH --job-name=sycl-weitere
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

OUT="$ROOT/ergebnisse_dachs/${SLURM_JOB_PARTITION:-lokal}_${SLURM_JOB_ID:-0}"
mkdir -p "$OUT"

{
  echo "== GPU";  nvidia-smi --query-gpu=name,driver_version --format=csv,noheader
  echo "== nvcc"; nvcc --version | tail -n 2
  echo "== DPC++"; clang++ --version | head -n 1
} > "$OUT/system.txt" 2>&1

echo "==> Matrixmultiplikation, natives CUDA"
SIZES="1024 2048" REPS=5 bash ../sycl-matmul/cuda/run_cuda.sh "$OUT/matmul_cuda.csv"

echo "==> Stencil und Reduktion (SYCL auf der GPU + CUDA)"
DEVICES="cuda:gpu" REPS=5 ./messen.sh "$OUT/stencil_reduktion.csv"

echo "Fertig: $OUT"
