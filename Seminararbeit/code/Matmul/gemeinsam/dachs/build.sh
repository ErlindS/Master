#!/usr/bin/env bash
# Alle Stufen und die Kachel-Varianten für die GPU bauen (und, falls eine
# OpenCL-CPU-Laufzeit vorhanden ist, für die CPU).
# Auf dem LOGIN-Knoten im Ordner Matmul ausführen:
#
#   source ~/sycl/env.sh
#   bash gemeinsam/dachs/build.sh               # NVIDIA (L40S, H100)
#   GPU=amd bash gemeinsam/dachs/build.sh       # AMD MI300A (braucht ROCm, siehe unten)
#
# Ist nvcc geladen, werden bei NVIDIA auch die CUDA-Gegenstücke gebaut.
#
# Für AMD vorher das ROCm-Modul laden, z. B.:
#   module avail 2>&1 | grep -i rocm
#   module load <rocm-modul>
# und das Modul in ~/sycl/env.sh eintragen, damit es auch im Job geladen wird.

#
# BUILD_DIR=<ordner>: dorthin bauen statt nach bin/ (nutzt alle_gpus.sh,
# damit NVIDIA- und AMD-Build nebeneinander existieren können).

set -euo pipefail
cd "$(dirname "$0")/../.."   # Ordner Matmul
mkdir -p ergebnisse/dachs   # Slurm braucht den Ordner für --output schon beim Abschicken

: "${DPCPP_HOME:?Erst 'source ~/sycl/env.sh' ausführen}"
GPU="${GPU:-nvidia}"

BIN="${BUILD_DIR:-$PWD/bin}"

case "$GPU" in
  nvidia)
    # nvptx64: PTX wird beim ersten Lauf für L40S/H100 just in time übersetzt
    TARGETS="${TARGETS:-nvptx64-nvidia-cuda,spir64}"
    EXTRA_FLAGS=""
    [ -n "${CUDA_HOME:-}" ] && EXTRA_FLAGS="--cuda-path=$CUDA_HOME"
    ;;
  amd)
    # amdgcn: AMD-GPUs werden nicht just in time übersetzt, die Architektur
    # muss beim Bauen feststehen. MI300A = gfx942.
    ARCH="${ARCH:-gfx942}"
    ROCM="${ROCM_PATH:-${ROCM_HOME:-}}"
    [ -n "$ROCM" ] || { echo "Fehler: ROCM_PATH nicht gesetzt – erst das ROCm-Modul laden." >&2; exit 1; }
    ls "$DPCPP_HOME/lib" | grep -q '^libur_adapter_hip\.so' \
      || echo "Warnung: libur_adapter_hip.so fehlt in $DPCPP_HOME/lib – DPC++ findet die AMD-GPU dann nicht." >&2
    TARGETS="${TARGETS:-amdgcn-amd-amdhsa,spir64}"
    EXTRA_FLAGS="-Xsycl-target-backend=amdgcn-amd-amdhsa --offload-arch=$ARCH --rocm-path=$ROCM"
    ;;
  *) echo "Unbekanntes GPU=$GPU (nvidia oder amd)" >&2; exit 2 ;;
esac

make BIN="$BIN" clean
make BIN="$BIN" CXX="$DPCPP_HOME/bin/clang++" SYCL_TARGETS="$TARGETS" EXTRA="$EXTRA_FLAGS" all kacheln
if [ "$GPU" = nvidia ] && command -v nvcc >/dev/null; then
  # sm_89 = L40S, sm_90 = H100; der Login-Knoten hat keine GPU für -arch=native
  make BIN="$BIN" ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90" cuda cuda-kacheln \
    || echo "Warnung: CUDA-Gegenstücke nicht gebaut." >&2
fi
echo "$GPU" > "$BIN/.gebaut_fuer"  # merkt sich, wofür gebaut wurde (prüft messung.sbatch)
echo "Fertig gebaut für $GPU ($TARGETS)."
