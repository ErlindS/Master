#!/usr/bin/env bash
# Läuft im Container: baut alle Stufen mit AdaptiveCpp und führt die
# Benchmarks auf allen verfügbaren Geräten aus.
#
# Erwartet:
#   /work/sycl-matmul   Quellcode (schreibgeschützt eingebunden)
#   /ergebnisse         Ausgabeordner (beschreibbar eingebunden)
#
# SIZES, REPS, TILES, SEQ_MAX werden an die Benchmark-Skripte durchgereicht.

set -euo pipefail

SRC=/work/sycl-matmul
OUT=${OUT:-/ergebnisse}
mkdir -p "$OUT"

# Quellcode in ein beschreibbares Verzeichnis kopieren und dort bauen
BUILD=$(mktemp -d)
cp -r "$SRC/." "$BUILD"
cd "$BUILD"

# Geräte erkennen: CPU (OpenMP) immer, NVIDIA-GPU nur mit --gpus all
DEVICES="omp"
if command -v nvidia-smi >/dev/null 2>&1 && nvidia-smi -L >/dev/null 2>&1; then
  DEVICES="omp cuda"
fi

{
  echo "Zeitpunkt: $(date -Iseconds)"
  echo "Geräte (ACPP_VISIBILITY_MASK): $DEVICES"
  echo
  acpp --acpp-version 2>&1 || true
  echo
  acpp-info -l 2>&1 || true
  echo
  nvidia-smi -L 2>/dev/null || true
  lscpu 2>/dev/null | grep -E 'Model name|^CPU\(s\)' || true
} > "$OUT/system.txt"

echo "==> Baue alle Stufen mit AdaptiveCpp"
make clean >/dev/null
make COMPILER=acpp all kacheln

for dev in $DEVICES; do
  echo "==> Stufen 0-5 auf Gerät: $dev"
  ACPP_VISIBILITY_MASK=$dev bash run_benchmark.sh "$OUT/stufen_$dev.csv"
done

echo "==> Kachelgrößen-Experiment"
SELECTOR_VAR=ACPP_VISIBILITY_MASK DEVICES="$DEVICES" bash run_kacheln.sh "$OUT/kacheln.csv"
python3 portabilitaet.py "$OUT/kacheln.csv" | tee "$OUT/portabilitaet.txt"

echo
echo "==> Fertig. Ergebnisse: $(ls "$OUT" | tr '\n' ' ')"
