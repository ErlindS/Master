#!/usr/bin/env bash
# Ein Befehl für alle Messungen der Seminararbeit auf diesem Rechner:
# bauen, alle Stufen (0, 1, 2, 2b, 3, 4, 5) auf jedem gefundenen Gerät messen,
# Kachelgrößen-Experiment inkl. Stufe 5, Performance-Portabilität auswerten.
#
#   ./messen.sh                         alles (n = 256 … 2048, 5 Wiederholungen)
#   SIZES="512 1024" REPS=3 ./messen.sh schneller Probelauf
#   DEVICES="cuda:gpu" ./messen.sh      nur bestimmte Geräte
#   COMPILER=acpp ./messen.sh           AdaptiveCpp statt DPC++ (nicht für die Arbeit)
#
# Voraussetzung DPC++: eine der folgenden Umgebungen ist vorhanden
#   ~/sycl/env.sh (dachs/install_dpcpp.sh), /opt/intel/oneapi/setvars.sh,
#   oder icpx / clang++ mit -fsycl im PATH.
#
# Ergebnis: ergebnisse/<datum>_<rechner>/
#   system.txt         Rechner, Geräte, Compiler
#   stufen_<gerät>.csv alle Stufen je Gerät (Tabellen "Leistung/Laufzeit je Stufe")
#   kacheln.csv        Stufe 4 je Kachelgröße + Stufe 5 (Zeile tile = auto)
#   portabilitaet.txt  Effizienz und PP je Kachelgröße und für Stufe 5
#   details.log        Meldungen der Programme, u. a. welche Kachelgröße
#                      Stufe 5 auf welchem Gerät gewählt hat
#
# Auf DACHS stattdessen:  bash dachs/build.sh && sbatch dachs/messung.sbatch

set -uo pipefail
cd "$(dirname "$0")"

COMPILER="${COMPILER:-dpcpp}"
export SIZES="${SIZES:-256 512 1024 2048}"
export REPS="${REPS:-5}"
export TILES="${TILES:-8 16 32 64}"

info() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33mWarnung:\033[0m %s\n' "$*" >&2; }
fail() { printf '\033[1;31mFehler:\033[0m %s\n' "$*" >&2; exit 1; }

# ---------------------------------------------------------------------------
# 1. Compiler finden
# ---------------------------------------------------------------------------
if [ "$COMPILER" = "acpp" ]; then
  command -v acpp >/dev/null || fail "acpp nicht im PATH"
  CXX_BIN="acpp"
  SELECTOR_VAR="ACPP_VISIBILITY_MASK"
  LIST_DEVICES() { acpp-info -l 2>/dev/null; }
  MAKE_ARGS=(COMPILER=acpp)
else
  if ! command -v sycl-ls >/dev/null; then
    if [ -f "$HOME/sycl/env.sh" ]; then
      # shellcheck disable=SC1091
      source "$HOME/sycl/env.sh"
    elif [ -f /opt/intel/oneapi/setvars.sh ]; then
      # shellcheck disable=SC1091
      source /opt/intel/oneapi/setvars.sh >/dev/null
    fi
  fi
  if [ -n "${DPCPP_HOME:-}" ] && [ -x "$DPCPP_HOME/bin/clang++" ]; then
    CXX_BIN="$DPCPP_HOME/bin/clang++"
  elif command -v icpx >/dev/null; then
    CXX_BIN="icpx"
  elif command -v clang++ >/dev/null && echo 'int main(){}' | clang++ -fsycl -x c++ - -o /dev/null 2>/dev/null; then
    CXX_BIN="clang++"
  else
    fail "Kein DPC++ gefunden. Erst installieren (z. B. bash dachs/install_dpcpp.sh) oder ~/sycl/env.sh bzw. /opt/intel/oneapi/setvars.sh bereitstellen."
  fi
  command -v sycl-ls >/dev/null || fail "sycl-ls nicht gefunden – DPC++-Umgebung unvollständig"
  SELECTOR_VAR="ONEAPI_DEVICE_SELECTOR"
  LIST_DEVICES() { sycl-ls 2>/dev/null; }
  MAKE_ARGS=(CXX="$CXX_BIN")
fi
info "Compiler: $CXX_BIN"

# ---------------------------------------------------------------------------
# 2. Geräte erkennen
# ---------------------------------------------------------------------------
GERAETE_LISTE="$(LIST_DEVICES)"
if [ -z "${DEVICES:-}" ]; then
  DEVICES=""
  if [ "$COMPILER" = "acpp" ]; then
    echo "$GERAETE_LISTE" | grep -qi "openmp" && DEVICES="omp"
    echo "$GERAETE_LISTE" | grep -qi "cuda"   && DEVICES="$DEVICES cuda"
  else
    echo "$GERAETE_LISTE" | grep -q "opencl:cpu" && DEVICES="opencl:cpu"
    echo "$GERAETE_LISTE" | grep -q "cuda:gpu"   && DEVICES="$DEVICES cuda:gpu"
    echo "$GERAETE_LISTE" | grep -q "hip:gpu"    && DEVICES="$DEVICES hip:gpu"
    echo "$GERAETE_LISTE" | grep -q "level_zero:gpu" && DEVICES="$DEVICES level_zero:gpu"
  fi
  DEVICES="$(echo $DEVICES)"   # Leerzeichen bereinigen
fi
[ -n "$DEVICES" ] || fail "Keine SYCL-Geräte gefunden. Ausgabe der Geräteliste:\n$GERAETE_LISTE"
info "Geräte: $DEVICES"

# ---------------------------------------------------------------------------
# 3. Bauen (Zwischendarstellungen passend zu den gefundenen Geräten)
# ---------------------------------------------------------------------------
if [ "$COMPILER" != "acpp" ]; then
  TARGETS="spir64"
  case " $DEVICES " in *" cuda:gpu "*) TARGETS="nvptx64-nvidia-cuda,$TARGETS" ;; esac
  case " $DEVICES " in *" hip:gpu "*)
    warn "AMD-GPU erkannt: Architektur über EXTRA angeben, z. B. EXTRA=\"-Xsycl-target-backend=amdgcn-amd-amdhsa --offload-arch=gfx942\""
    TARGETS="amdgcn-amd-amdhsa,$TARGETS" ;;
  esac
  CUDA_FLAG=""
  [ -n "${CUDA_HOME:-}" ] && CUDA_FLAG="--cuda-path=$CUDA_HOME"
  MAKE_ARGS+=(SYCL_TARGETS="$TARGETS" EXTRA="$CUDA_FLAG ${EXTRA:-}")
fi

info "Baue alle Stufen und Kachel-Varianten (TILES=$TILES)"
make clean >/dev/null
make -j"$(nproc 2>/dev/null || echo 4)" "${MAKE_ARGS[@]}" TILES="$TILES" all kacheln \
  || fail "Bauen fehlgeschlagen (Ausgabe oben)"

# ---------------------------------------------------------------------------
# 4. Messen
# ---------------------------------------------------------------------------
OUT="ergebnisse/$(date +%Y-%m-%d_%H%M)_$(hostname -s)"
mkdir -p "$OUT"
LOG="$OUT/details.log"

{
  echo "== Rechner";   hostname; date
  echo "== CPU";       lscpu 2>/dev/null | grep -E 'Model name|^CPU\(s\)'
  echo "== GPU";       nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv 2>/dev/null || echo "keine NVIDIA-GPU / nvidia-smi"
  echo "== Compiler";  "$CXX_BIN" --version 2>/dev/null | head -1
  echo "== Geräte";    echo "$GERAETE_LISTE"
  echo "== Einstellungen"; echo "SIZES=$SIZES REPS=$REPS TILES=$TILES DEVICES=$DEVICES"
} > "$OUT/system.txt"

erstes=1
for dev in $DEVICES; do
  name="${dev//:/_}"
  info "Alle Stufen auf $dev"
  # Stufe 0 (sequentiell, ohne SYCL) nur beim ersten Gerät messen
  seq_max="${SEQ_MAX:-1024}"; [ $erstes -eq 1 ] || seq_max=0
  echo "### Stufen auf $dev" >> "$LOG"
  env "$SELECTOR_VAR=$dev" SEQ_MAX="$seq_max" bash run_benchmark.sh "$OUT/stufen_$name.csv" 2>>"$LOG" \
    || warn "Stufen auf $dev nicht vollständig durchgelaufen (siehe $LOG)"
  erstes=0
done

info "Kachelgrößen-Experiment (Stufe 4) und Stufe 5"
echo "### Kachelexperiment" >> "$LOG"
SELECTOR_VAR="$SELECTOR_VAR" DEVICES="$DEVICES" SIZES="${KACHEL_SIZES:-1024 2048}" \
  bash run_kacheln.sh "$OUT/kacheln.csv" 2>>"$LOG"

# Stufe 5 schreibt ihre Auswahl nach stderr, run_kacheln.sh verwirft das.
# Daher einmal je Gerät mit sichtbarer Auswahl für details.log:
for dev in $DEVICES; do
  echo "### Auswahl Stufe 5 auf $dev (n = 1024)" >> "$LOG"
  env "$SELECTOR_VAR=$dev" ./stufe5_anpassung 1024 1 >/dev/null 2>>"$LOG"
done

python3 portabilitaet.py "$OUT/kacheln.csv" > "$OUT/portabilitaet.txt" \
  || warn "Auswertung fehlgeschlagen"

# ---------------------------------------------------------------------------
# 5. Zusammenfassung
# ---------------------------------------------------------------------------
echo
info "Fertig. Ergebnisse in $OUT/"
ls -1 "$OUT" | sed 's/^/    /'
echo
if grep -q ";NEIN$" "$OUT"/stufen_*.csv 2>/dev/null; then
  warn "Mindestens eine Stufe hat ein falsches Ergebnis geliefert:"
  grep -h ";NEIN$" "$OUT"/stufen_*.csv
fi
echo "Gewählte Kachelgrößen in Stufe 5 (n = 1024):"
awk '/^### Auswahl Stufe 5 auf/ {dev=$6} dev && /gewählt/ {print "    " dev ": " $0}' "$LOG"
echo
tail -n 6 "$OUT/portabilitaet.txt"
