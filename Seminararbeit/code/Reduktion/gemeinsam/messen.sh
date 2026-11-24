#!/usr/bin/env bash
# Reduktion (Summe von n Floats): alle Varianten auf allen Geräten messen (SYCL + natives CUDA).
# Im Ordner Reduktion:
#
#   source ~/sycl/env.sh
#   make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA=--cuda-path=$CUDA_HOME
#   make cuda
#   bash gemeinsam/messen.sh                  # -> ergebnisse/<datum>_<rechner>.csv
#
# Einstellungen: DEVICES="cuda:gpu opencl:cpu", REPS=5, SIZES="16777216 67108864",
#   BIN_DIR=bin (Ordner mit den gebauten Programmen),
#   VARIANTEN="reduktion5_gemeinsam reduktion6_buffer" (nur diese messen),
#   GRUPPEN="64 256 1024" (Gruppengrößen für reduktion7_geraet, leer = überspringen),
#   SELECTOR_VAR=ONEAPI_DEVICE_SELECTOR (AdaptiveCpp: ACPP_VISIBILITY_MASK, DEVICES="hip omp")

set -uo pipefail
cd "$(dirname "$0")/.."   # Ordner Reduktion

DEVICES="${DEVICES:-cuda:gpu opencl:cpu}"
REPS="${REPS:-5}"
SIZES="${SIZES:-16777216 67108864}"
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

# Die Stufen-Gegenstücke stehen in derselben Reihenfolge wie die Stufen der
# Matrixmultiplikation: 5 = Stufe 1 (gemeinsamer Speicher), 6 = Stufe 2b
# (Buffer), 1 bis 4 = Gerätespeicher mit steigender Gruppenarbeit,
# 7 = Stufe 5 (Gruppengröße vom Gerät).
GRUPPEN="${GRUPPEN:-64 256 1024}"   # Gruppengrößen W für reduktion7_geraet, leer = überspringen
VARIANTEN="${VARIANTEN:-reduktion5_gemeinsam reduktion6_buffer reduktion6_buffer_wieder \
                        reduktion1_atomar reduktion2_lokal reduktion3_gruppe \
                        reduktion4_builtin reduktion7_geraet}"

for dev in $DEVICES; do
  for n in $SIZES; do
    for b in $VARIANTEN; do run "$dev" "$B/$b" "$n"; done
    for w in $GRUPPEN; do run "$dev" "$B/reduktion7_geraet" "$n" "$w"; done
  done
done

if command -v nvidia-smi >/dev/null && [ -x "$B/cuda_reduktion_atomar" ]; then
  for n in $SIZES; do
    for b in cuda_reduktion_atomar cuda_reduktion_shared cuda_reduktion_cub; do run cuda "$B/$b" "$n"; done
  done
fi

echo "Fertig: $OUT"
