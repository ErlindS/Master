#!/bin/bash -l
# Versuchsaufbau (H1–H5) auf einem DACHS-GPU-Knoten.
# Gebaut wird IM Job auf dem GPU-Knoten, damit -arch=native (nvcc) und
# -march=native (g++) die richtige GPU/CPU treffen. Im Ordner versuchsaufbau:
#
#   sbatch -p gpu1 dachs.sbatch                  # L40S
#   sbatch -p gpu8 dachs.sbatch                  # H100
#   RUNS=2 REPS=5 SIZES="512 1024" sbatch dachs.sbatch   # Probelauf
#
# Ergebnis: messdaten/dachs_<partition>_<jobid>.csv (+ _system.txt, .log)
#
#SBATCH --job-name=sycl-versuch
#SBATCH --partition=gpu1
#SBATCH --gres=gpu:1
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=32G
#SBATCH --time=02:00:00
#SBATCH --output=messdaten/slurm_%j.out

set -uo pipefail
cd "${SLURM_SUBMIT_DIR:-.}"
source "$HOME/sycl/env.sh"
# OpenBLAS für cpu_blas, falls als Modul vorhanden (sonst wird cpu_blas übersprungen)
module load numlib/openblas 2>/dev/null || true

export DPCPP_CXX="$DPCPP_HOME/bin/clang++"
export OMP_NUM_THREADS="${SLURM_CPUS_PER_TASK:-8}"
export OMP_PROC_BIND=close OMP_PLACES=cores

echo "==> Bauen auf $(hostname)"
make clean
make dpcpp DPCPP_CXX="$DPCPP_CXX" DPCPP_EXTRA="${CUDA_HOME:+--cuda-path=$CUDA_HOME}"
make cuda
make bin/cpu_triad
make bin/cpu_blas || echo "cpu_blas nicht gebaut (kein OpenBLAS) – wird übersprungen"

# Auf DACHS gibt es kein OpenCL-CPU-Gerät -> SYCL nur auf der GPU
DPCPP_GERAETE="${DPCPP_GERAETE:-cuda:gpu}" \
  ./run_all.sh "messdaten/dachs_${SLURM_JOB_PARTITION:-lokal}_${SLURM_JOB_ID:-0}.csv"
