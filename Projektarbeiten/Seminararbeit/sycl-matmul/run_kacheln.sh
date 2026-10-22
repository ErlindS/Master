#!/usr/bin/env bash
# Kachelgrößen-Experiment: Portabilität ≠ Performance-Portabilität
#
# Derselbe Quellcode (stufe3_tiling.cpp) wird mit mehreren Kachelgrößen
# gebaut und auf jedem Gerät ausgeführt. Ergebnis: eine CSV, die
# portabilitaet.py auswertet.
#
#   make kacheln                     # vorher bauen (TILES wie unten)
#   ./run_kacheln.sh                 # -> kacheln.csv
#   python3 portabilitaet.py kacheln.csv
#
# Einstellungen über Umgebungsvariablen:
#   TILES="8 16 32 64"               Kachelgrößen
#   DEVICES="opencl:cpu cuda:gpu"    Geräte (Werte für SELECTOR_VAR)
#   SIZES="1024 2048"                Matrixgrößen
#   REPS=5                           Messwiederholungen
#   SELECTOR_VAR=ONEAPI_DEVICE_SELECTOR   (AdaptiveCpp: ACPP_VISIBILITY_MASK,
#                                          DEVICES="omp cuda")

set -uo pipefail

OUT="${1:-kacheln.csv}"
TILES="${TILES:-8 16 32 64}"
DEVICES="${DEVICES:-opencl:cpu cuda:gpu}"
SIZES="${SIZES:-1024 2048}"
REPS="${REPS:-5}"
SELECTOR_VAR="${SELECTOR_VAR:-ONEAPI_DEVICE_SELECTOR}"

echo "auswahl;geraet;tile;n;median_ms;gflops;status" > "$OUT"

for dev in $DEVICES; do
  for n in $SIZES; do
    for t in $TILES; do
      bin="./stufe3_tiling_T$t"
      if [ ! -x "$bin" ]; then
        echo "$bin fehlt – erst 'make kacheln' ausführen" >&2
        exit 1
      fi
      # Ausgabe von bench::report:
      #   stufe;geraet;n;erster_lauf_ms;median_ms;transfer_ms;gflops;korrekt
      if line=$(env "$SELECTOR_VAR=$dev" "$bin" "$n" "$REPS" 2>/dev/null); then
        IFS=';' read -r _ name _ _ med _ gf ok <<< "$line"
        name="$(echo "$name" | xargs)"   # Leerzeichen am Ende entfernen
        status=$([ "$ok" = "ja" ] && echo ok || echo falsch)
        echo "$dev;$name;$t;$n;$med;$gf;$status" | tee -a "$OUT"
      else
        # z. B. Work-Group größer als das Gerät erlaubt -> Kernel startet nicht
        echo "$dev;;$t;$n;;;startet_nicht" | tee -a "$OUT"
      fi
    done
  done
done

echo "Fertig: $OUT"
