#!/usr/bin/env bash
# Stencil und Reduktion auf allen Geräten messen (SYCL + natives CUDA).
#
#   source ~/sycl/env.sh
#   make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA=--cuda-path=$CUDA_HOME
#   make -C cuda
#   ./messen.sh                                    # -> ergebnisse/<datum>_<rechner>.csv
#
# Einstellungen: DEVICES="cuda:gpu opencl:cpu", REPS=5,
#   STENCIL_SIZES="4096 8192", RED_SIZES="16777216 67108864"

set -uo pipefail
cd "$(dirname "$0")"

DEVICES="${DEVICES:-cuda:gpu opencl:cpu}"
REPS="${REPS:-5}"
STENCIL_SIZES="${STENCIL_SIZES:-4096 8192}"
RED_SIZES="${RED_SIZES:-16777216 67108864}"
mkdir -p ergebnisse
OUT="${1:-ergebnisse/$(date +%Y-%m-%d)_$(hostname -s).csv}"

echo "variante;geraet;n;erster_lauf_ms;median_ms;gb_s;korrekt;selector" > "$OUT"

run() {   # run <selector|cuda> <programm> <n>
  local sel="$1" bin="$2" n="$3" line
  if [ "$sel" = "cuda" ]; then
    line=$("$bin" "$n" "$REPS" 2>/dev/null)
  else
    line=$(ONEAPI_DEVICE_SELECTOR="$sel" "$bin" "$n" "$REPS" 2>/dev/null)
  fi
  if [ -n "$line" ]; then echo "$line;$sel" | tee -a "$OUT"
  else echo "$(basename "$bin");;$n;;;;startet_nicht;$sel" | tee -a "$OUT"; fi
}

for dev in $DEVICES; do
  for n in $STENCIL_SIZES; do
    for b in stencil1_2d stencil1_2d_sync stencil2_1d stencil3_lokal; do run "$dev" "./$b" "$n"; done
  done
  for n in $RED_SIZES; do
    for b in reduktion1_atomar reduktion2_lokal reduktion3_gruppe reduktion4_builtin; do run "$dev" "./$b" "$n"; done
  done
done

if command -v nvidia-smi >/dev/null && [ -x cuda/cuda_stencil_global ]; then
  for n in $STENCIL_SIZES; do
    for b in cuda_stencil_global cuda_stencil_shared; do run cuda "cuda/$b" "$n"; done
  done
  for n in $RED_SIZES; do
    for b in cuda_reduktion_atomar cuda_reduktion_shared cuda_reduktion_cub; do run cuda "cuda/$b" "$n"; done
  done
fi

echo "Fertig: $OUT"
