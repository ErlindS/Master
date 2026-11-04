#!/usr/bin/env bash
# Alle Stufen und die Kachel-Varianten für NVIDIA-GPUs (und, falls eine
# OpenCL-CPU-Laufzeit vorhanden ist, für die CPU) bauen.
# Auf dem LOGIN-Knoten im Ordner sycl-matmul ausführen:
#
#   source ~/sycl/env.sh
#   bash dachs/build.sh

set -euo pipefail
cd "$(dirname "$0")/.."

: "${DPCPP_HOME:?Erst 'source ~/sycl/env.sh' ausführen}"

# nvptx64: NVIDIA (PTX wird beim ersten Lauf für L40S/H100 just in time übersetzt)
# spir64:  generischer Code für OpenCL-Geräte (z. B. CPU)
TARGETS="${TARGETS:-nvptx64-nvidia-cuda,spir64}"
CUDA_FLAG=""
[ -n "${CUDA_HOME:-}" ] && CUDA_FLAG="--cuda-path=$CUDA_HOME"

make clean
make CXX="$DPCPP_HOME/bin/clang++" SYCL_TARGETS="$TARGETS" EXTRA="$CUDA_FLAG" all kacheln
mkdir -p ergebnisse_dachs   # Slurm braucht den Ordner für --output schon beim Abschicken
echo "Fertig gebaut."
