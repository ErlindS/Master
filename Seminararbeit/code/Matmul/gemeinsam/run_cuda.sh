#!/usr/bin/env bash
# Alle CUDA-Vergleichsprogramme messen, gleiches CSV-Format wie die SYCL-Stufen.
#
#   make cuda cuda-kacheln                       (im Ordner Matmul)
#   bash gemeinsam/run_cuda.sh                   # -> cuda.csv
#   SIZES="1024 2048" REPS=5 bash gemeinsam/run_cuda.sh ergebnis.csv
#
# Kachelvarianten, die nicht starten (T = 64), erscheinen mit korrekt = startet_nicht.

set -uo pipefail
OUT="${1:-cuda.csv}"
B="${BIN_DIR:-$(cd "$(dirname "$0")/.." && pwd)/bin}"   # gebaute Programme (make -> Matmul/bin)
SIZES="${SIZES:-1024 2048}"
REPS="${REPS:-5}"
TILES="${TILES:-8 16 32 64}"

echo "stufe;geraet;n;erster_lauf_ms;median_ms;transfer_ms;gflops;korrekt" > "$OUT"

run() {   # run <programm> <n> <name falls Start scheitert>
  if line=$("$B/$1" "$2" "$REPS" 2>/dev/null); then
    echo "$line" | tee -a "$OUT"
  else
    echo "$3;;$2;;;;;startet_nicht" | tee -a "$OUT"
  fi
}

for n in $SIZES; do
  for p in cuda1_managed cuda2_device cuda4_tiling cuda_cublas; do
    [ -x "$B/$p" ] && run "$p" "$n" "$p"
  done
  for t in $TILES; do
    [ -x "$B/cuda4_tiling_T$t" ] && run "cuda4_tiling_T$t" "$n" "cuda_4_tiling_T$t"
  done
done

echo "Fertig: $OUT"
