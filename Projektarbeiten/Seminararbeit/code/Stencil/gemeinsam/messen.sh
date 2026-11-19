#!/usr/bin/env bash
# 2D-Stencil (Wärmeleitung, Jacobi, 5 Punkte): alle Varianten auf allen Geräten messen (SYCL + natives CUDA).
# Im Ordner Stencil:
#
#   source ~/sycl/env.sh
#   make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA=--cuda-path=$CUDA_HOME
#   make cuda
#   bash gemeinsam/messen.sh                  # -> ergebnisse/<datum>_<rechner>.csv
#
# Einstellungen: DEVICES="cuda:gpu opencl:cpu", REPS=5, SIZES="4096 8192",
#   BIN_DIR=bin (Ordner mit den gebauten Programmen),
#   VARIANTEN="stencil4_gemeinsam stencil5_buffer" (nur diese messen),
#   GRUPPEN="8 16 32" (Gruppengrößen für stencil6_gruppe, leer = überspringen),
#   SELECTOR_VAR=ONEAPI_DEVICE_SELECTOR (AdaptiveCpp: ACPP_VISIBILITY_MASK, DEVICES="hip omp")

set -uo pipefail
cd "$(dirname "$0")/.."   # Ordner Stencil

DEVICES="${DEVICES:-cuda:gpu opencl:cpu}"
REPS="${REPS:-5}"
SIZES="${SIZES:-4096 8192}"
B="${BIN_DIR:-bin}"
SELECTOR_VAR="${SELECTOR_VAR:-ONEAPI_DEVICE_SELECTOR}"
mkdir -p ergebnisse
OUT="${1:-ergebnisse/$(date +%Y-%m-%d)_$(hostname -s).csv}"

echo "variante;geraet;n;erster_lauf_ms;median_ms;gb_s;korrekt;selector" > "$OUT"

run() {   # run <selector|cuda> <programm> <n> [parameter]
  local sel="$1" bin="$2" n="$3" par="${4:-}" line
  if [ "$sel" = "cuda" ]; then
    line=$("$bin" "$n" "$REPS" $par 2>/dev/null)
  else
    line=$(env "$SELECTOR_VAR=$sel" "$bin" "$n" "$REPS" $par 2>/dev/null)
  fi
  if [ -n "$line" ]; then echo "$line;$sel" | tee -a "$OUT"
  else echo "$(basename "$bin");;$n;;;;startet_nicht;$sel" | tee -a "$OUT"; fi
}

# Varianten ohne Parameter. Die Stufen-Gegenstücke (4 bis 7) stehen dabei in
# derselben Reihenfolge wie die Stufen der Matrixmultiplikation:
#   4 = Stufe 1 (gemeinsamer Speicher), 5 = Stufe 2b (Buffer),
#   1 bis 3 = Stufe 2 bis 4, 7 = Stufe 5 (Kachelgröße vom Gerät)
GRUPPEN="${GRUPPEN:-8 16 32}"   # Gruppengrößen W für stencil6_gruppe, leer = überspringen
VARIANTEN="${VARIANTEN:-stencil4_gemeinsam stencil5_buffer stencil5_buffer_wieder \
                        stencil1_2d stencil1_2d_sync stencil2_1d stencil3_lokal \
                        stencil7_geraet stencil7_geraet_flach}"

for dev in $DEVICES; do
  for n in $SIZES; do
    for b in $VARIANTEN; do run "$dev" "$B/$b" "$n"; done
    for w in $GRUPPEN; do run "$dev" "$B/stencil6_gruppe" "$n" "$w"; done
  done
done

if command -v nvidia-smi >/dev/null && [ -x "$B/cuda_stencil_global" ]; then
  for n in $SIZES; do
    for b in cuda_stencil_global cuda_stencil_shared; do run cuda "$B/$b" "$n"; done
  done
fi

echo "Fertig: $OUT"
