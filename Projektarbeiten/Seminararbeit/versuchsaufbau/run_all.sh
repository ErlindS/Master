#!/usr/bin/env bash
# Führt den kompletten Versuchsplan aus und schreibt EINE Rohdaten-CSV.
#
#   make && ./run_all.sh                  -> messdaten/roh_<datum>.csv
#   RUNS=2 REPS=5 SIZES="512" ./run_all.sh   (schneller Probelauf)
#
# Versuchsplan:
#   * jede Konfiguration wird in RUNS unabhängigen Programmstarts gemessen
#     (je REPS Wiederholungen + erster Lauf), damit Streuung zwischen
#     Starts sichtbar wird und Konfidenzintervalle möglich sind
#   * die Reihenfolge der Konfigurationen wird pro Lauf zufällig gemischt,
#     damit Drift (Erwärmung, Taktraten) nicht systematisch eine Variante trifft
#   * fehlende Programme (z. B. kein AdaptiveCpp installiert) werden
#     übersprungen
#
# Einstellungen (Umgebungsvariablen):
#   RUNS=5  REPS=20  SIZES="256 512 1024 2048"  TRIAD_N="16777216 67108864"
#   VARIANTEN="naiv tile8 tile16 tile32 tile64 auto"
#   DPCPP_GERAETE="opencl:cpu cuda:gpu"   ACPP_GERAETE="omp cuda"
#   PAUSE=1 (Sekunden zwischen Messungen)   TIMEOUT=600   SEED=1

set -uo pipefail
cd "$(dirname "$0")"

STAMP=$(date +%Y%m%d_%H%M)
OUT="${1:-messdaten/roh_${STAMP}.csv}"
LOG="${OUT%.csv}.log"
RUNS="${RUNS:-5}"
REPS="${REPS:-20}"
SIZES="${SIZES:-256 512 1024 2048}"
TRIAD_N="${TRIAD_N:-16777216 67108864}"
VARIANTEN="${VARIANTEN:-naiv tile8 tile16 tile32 tile64 auto}"
DPCPP_GERAETE="${DPCPP_GERAETE:-opencl:cpu cuda:gpu}"
ACPP_GERAETE="${ACPP_GERAETE:-omp cuda}"
PAUSE="${PAUSE:-1}"
TIMEOUT="${TIMEOUT:-600}"
SEED="${SEED:-1}"

export NVIDIA_TF32_OVERRIDE=0   # cuBLAS: reines FP32

mkdir -p "$(dirname "$OUT")"
./systeminfo.sh > "${OUT%.csv}_system.txt"
echo "impl;auswahl;klasse;geraet;kernel;variante;n;lauf;wdh;zeit_ms;leistung;einheit;status" > "$OUT"
: > "$LOG"

# --- Jobliste: eine Zeile = Umgebungsvariablen + Programm + Argumente --------
JOBS=$(mktemp)
trap 'rm -f "$JOBS"' EXIT

vorhanden() { [ -x "bin/$1" ] || { echo "übersprungen: bin/$1 fehlt" >> "$LOG"; return 1; }; }

if vorhanden sycl_gemm_dpcpp; then
  for dev in $DPCPP_GERAETE; do
    for n in $SIZES; do
      for v in $VARIANTEN; do
        echo "AUSWAHL=$dev ONEAPI_DEVICE_SELECTOR=$dev bin/sycl_gemm_dpcpp $v $n $REPS" >> "$JOBS"
      done
    done
    for n in $TRIAD_N; do
      vorhanden sycl_triad_dpcpp && \
        echo "AUSWAHL=$dev ONEAPI_DEVICE_SELECTOR=$dev bin/sycl_triad_dpcpp triad $n $REPS" >> "$JOBS"
    done
  done
fi

if vorhanden sycl_gemm_acpp; then
  for dev in $ACPP_GERAETE; do
    for n in $SIZES; do
      for v in $VARIANTEN; do
        echo "AUSWAHL=$dev ACPP_VISIBILITY_MASK=$dev bin/sycl_gemm_acpp $v $n $REPS" >> "$JOBS"
      done
    done
    for n in $TRIAD_N; do
      vorhanden sycl_triad_acpp && \
        echo "AUSWAHL=$dev ACPP_VISIBILITY_MASK=$dev bin/sycl_triad_acpp triad $n $REPS" >> "$JOBS"
    done
  done
fi

if vorhanden cuda_gemm; then
  for n in $SIZES; do
    for v in naiv tile8 tile16 tile32 tile64 cublas; do
      echo "AUSWAHL=cuda bin/cuda_gemm $v $n $REPS" >> "$JOBS"
    done
  done
fi
if vorhanden cuda_triad; then
  for n in $TRIAD_N; do echo "AUSWAHL=cuda bin/cuda_triad triad $n $REPS" >> "$JOBS"; done
fi

if vorhanden cpu_blas; then
  for n in $SIZES; do echo "AUSWAHL=openblas bin/cpu_blas sgemm $n $REPS" >> "$JOBS"; done
fi
if vorhanden cpu_triad; then
  for n in $TRIAD_N; do echo "AUSWAHL=openmp bin/cpu_triad triad $n $REPS" >> "$JOBS"; done
fi

TOTAL=$(( $(wc -l < "$JOBS") * RUNS ))
echo "Versuchsplan: $(wc -l < "$JOBS") Konfigurationen x $RUNS Läufe = $TOTAL Programmstarts"
echo "Rohdaten: $OUT"

# --- Ausführung --------------------------------------------------------------
i=0
for lauf in $(seq 1 "$RUNS"); do
  # pro Lauf andere, aber reproduzierbare Reihenfolge
  while read -r job; do
    i=$((i + 1))
    printf '\r[%d/%d] Lauf %d: %-70.70s' "$i" "$TOTAL" "$lauf" "$job"
    # shellcheck disable=SC2086
    timeout "$TIMEOUT" env LAUF="$lauf" $job >> "$OUT" 2>> "$LOG"
    status=$?
    [ "$status" -ne 0 ] && echo "Status $status: LAUF=$lauf $job" >> "$LOG"
    sleep "$PAUSE"
  done < <(shuf --random-source=<(yes "$SEED$lauf") "$JOBS")
done
echo
echo "fertig. Auswertung:  python3 auswertung.py $OUT"
