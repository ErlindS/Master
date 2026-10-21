#!/usr/bin/env bash
# Führt alle Stufen für mehrere Matrixgrößen aus und schreibt eine CSV.
#
#   ./run_benchmark.sh                     -> Standardgerät, ergebnisse.csv
#   ./run_benchmark.sh gpu_nvidia.csv      -> eigener Dateiname
#
# Gerät wählen (Stufe 4: derselbe Code auf verschiedener Hardware):
#   DPC++:        ONEAPI_DEVICE_SELECTOR=cuda:gpu   ./run_benchmark.sh nvidia.csv
#                 ONEAPI_DEVICE_SELECTOR=opencl:cpu ./run_benchmark.sh cpu.csv
#                 ONEAPI_DEVICE_SELECTOR=level_zero:gpu ./run_benchmark.sh intel_gpu.csv
#   AdaptiveCpp:  ACPP_VISIBILITY_MASK=cuda ./run_benchmark.sh nvidia.csv
#                 ACPP_VISIBILITY_MASK=omp  ./run_benchmark.sh cpu.csv
#   Verfügbare Geräte anzeigen:  sycl-ls  bzw.  acpp-info

set -euo pipefail

OUT="${1:-ergebnisse.csv}"
SIZES="${SIZES:-256 512 1024 2048}"
SEQ_MAX="${SEQ_MAX:-1024}"   # Stufe 0 ist bei großen n sehr langsam
REPS="${REPS:-5}"

echo "stufe;geraet;n;erster_lauf_ms;median_ms;transfer_ms;gflops;korrekt" > "$OUT"

for n in $SIZES; do
  if [ "$n" -le "$SEQ_MAX" ]; then
    ./stufe0_sequentiell "$n" "$REPS" | tee -a "$OUT"
  fi
  ./stufe1_usm_shared "$n" "$REPS" | tee -a "$OUT"
  ./stufe2_usm_device "$n" "$REPS" | tee -a "$OUT"
  ./stufe3_tiling     "$n" "$REPS" | tee -a "$OUT"
done

echo "Fertig: $OUT"
