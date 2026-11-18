#!/bin/bash -l
# DACHS-Job: 2D-Stencil (Wärmeleitung, Jacobi, 5 Punkte) in SYCL (GPU) und CUDA.
#
# Vorher auf dem Login-Knoten im Ordner Stencil bauen:
#   source ~/sycl/env.sh
#   make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA="--cuda-path=$CUDA_HOME"
#   make cuda ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90"
#   mkdir -p ergebnisse/dachs
# dann im Ordner Stencil abschicken:
#   sbatch -p gpu1 gemeinsam/dachs.sbatch     # L40S
#   sbatch -p gpu8 gemeinsam/dachs.sbatch     # H100
#
# Ergebnis: ergebnisse/dachs/<partition>_<jobid>/
#
#SBATCH --job-name=sycl-stencil
#SBATCH --partition=gpu1
#SBATCH --gres=gpu:1
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=32G
#SBATCH --time=01:00:00
#SBATCH --output=ergebnisse/dachs/slurm_%j.out

set -uo pipefail
ROOT="$(cd "${SLURM_SUBMIT_DIR:-.}" && pwd)"
cd "$ROOT"
source "$HOME/sycl/env.sh"

OUT="$ROOT/ergebnisse/dachs/${SLURM_JOB_PARTITION:-lokal}_${SLURM_JOB_ID:-0}"
mkdir -p "$OUT"

{
  echo "== GPU";  nvidia-smi --query-gpu=name,driver_version --format=csv,noheader
  echo "== nvcc"; nvcc --version | tail -n 2
  echo "== DPC++"; clang++ --version | head -n 1
} > "$OUT/system.txt" 2>&1

DEVICES="cuda:gpu" REPS=5 bash gemeinsam/messen.sh "$OUT/stencil.csv"

echo "Fertig: $OUT"
