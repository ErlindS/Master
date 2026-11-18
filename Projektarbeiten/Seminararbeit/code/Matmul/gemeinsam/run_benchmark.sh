#!/usr/bin/env bash
# Führt alle Stufen für mehrere Matrixgrößen aus und schreibt eine CSV.
#
#   bash gemeinsam/run_benchmark.sh                  -> Standardgerät, ergebnisse.csv
#   bash gemeinsam/run_benchmark.sh gpu_nvidia.csv   -> eigener Dateiname
#   Programme werden in Matmul/bin erwartet (make), anderer Ordner: BIN_DIR=...
#
# Gerät wählen (derselbe Code auf verschiedener Hardware):
#   DPC++:        ONEAPI_DEVICE_SELECTOR=cuda:gpu   ./run_benchmark.sh nvidia.csv
#                 ONEAPI_DEVICE_SELECTOR=opencl:cpu ./run_benchmark.sh cpu.csv
#                 ONEAPI_DEVICE_SELECTOR=level_zero:gpu ./run_benchmark.sh intel_gpu.csv
#   AdaptiveCpp:  ACPP_VISIBILITY_MASK=cuda ./run_benchmark.sh nvidia.csv
#                 ACPP_VISIBILITY_MASK=omp  ./run_benchmark.sh cpu.csv
#   Verfügbare Geräte anzeigen:  sycl-ls  bzw.  acpp-info

set -euo pipefail

OUT="${1:-ergebnisse.csv}"
B="${BIN_DIR:-$(cd "$(dirname "$0")/.." && pwd)/bin}"   # gebaute Programme (make -> Matmul/bin)
SIZES="${SIZES:-256 512 1024 2048}"
SEQ_MAX="${SEQ_MAX:-1024}"   # Stufe 0 ist bei großen n sehr langsam
REPS="${REPS:-5}"

echo "stufe;geraet;n;erster_lauf_ms;median_ms;transfer_ms;gflops;korrekt" > "$OUT"

for n in $SIZES; do
  if [ "$n" -le "$SEQ_MAX" ]; then
    "$B/stufe0_sequentiell" "$n" "$REPS" | tee -a "$OUT"
  fi
  "$B/stufe1_usm_shared" "$n" "$REPS" | tee -a "$OUT"
  "$B/stufe2_usm_device" "$n" "$REPS" | tee -a "$OUT"
  "$B/stufe2b_buffer"    "$n" "$REPS" | tee -a "$OUT"
  "$B/stufe3_ndrange"    "$n" "$REPS" | tee -a "$OUT"
  "$B/stufe4_tiling"     "$n" "$REPS" | tee -a "$OUT"
  "$B/stufe5_anpassung"  "$n" "$REPS" | tee -a "$OUT"
done

echo "Fertig: $OUT"
