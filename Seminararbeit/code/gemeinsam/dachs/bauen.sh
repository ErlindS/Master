#!/usr/bin/env bash
# Alle SYCL- und CUDA-Programme auf dem DACHS-Login-Knoten bauen.
#
# Von lokal (ein einziger Befehl, nichts einfügen):
#   dachs bash gemeinsam/dachs/bauen.sh
# Auf DACHS:
#   cd ~/code && bash gemeinsam/dachs/bauen.sh
#
# Einstellungen:
#   BEISPIELE="Stencil Reduktion"   nur diese bauen (Vorgabe: alle drei)
#   ARCHE="-arch=native"            CUDA-Zielarchitekturen (Vorgabe: sm_89 + sm_90)
#   SYCL_ENV=~/sycl/env.sh          Umgebungsskript von install_dpcpp.sh
#   SYCL_CXX=clang++                SYCL-Compiler (Vorgabe: clang++ aus dem PATH)

set -uo pipefail
SELF="$(readlink -f "$0")"
cd "$(dirname "$SELF")/../.."   # Ordner code

# In einer nicht-interaktiven Shell (ssh <befehl>) ist "module" nicht geladen,
# env.sh braucht es aber für das CUDA-Modul.
if ! command -v module >/dev/null 2>&1; then
  for init in /etc/profile.d/modules.sh /etc/profile.d/z00_lmod.sh \
              /usr/share/lmod/lmod/init/bash; do
    # shellcheck disable=SC1090
    [ -r "$init" ] && . "$init" && break
  done
fi

ENV_SH="${SYCL_ENV:-$HOME/sycl/env.sh}"
if [ ! -r "$ENV_SH" ]; then
  echo "✗ $ENV_SH fehlt. DPC++ zuerst installieren:" >&2
  echo "    bash gemeinsam/dachs/install_dpcpp.sh" >&2
  exit 1
fi
# shellcheck disable=SC1090
. "$ENV_SH"

command -v clang++ >/dev/null 2>&1 || { echo "✗ clang++ nicht im PATH (env.sh?)" >&2; exit 1; }
if [ -z "${CUDA_HOME:-}" ]; then
  echo "! CUDA_HOME ist leer – das CUDA-Modul wurde nicht geladen." >&2
  echo "  SYCL wird trotzdem versucht, nvcc fehlt dann aber." >&2
fi
echo "DPC++: $(clang++ --version | head -n 1)"
echo "CUDA_HOME: ${CUDA_HOME:-<leer>}"
echo

BEISPIELE="${BEISPIELE:-Matmul Stencil Reduktion}"
ARCHE="${ARCHE:--gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90}"
fehler=0

# Der SYCL-Compiler: clang++ aus dem DPC++-Release. Matmul würde sonst icpx
# nehmen, das es auf DACHS nicht gibt.
SYCL_CXX="${SYCL_CXX:-$(command -v clang++)}"
echo "SYCL-Compiler: $SYCL_CXX"
echo

for b in $BEISPIELE; do
  [ -f "$b/Makefile" ] || { echo "✗ $b/Makefile fehlt"; fehler=1; continue; }
  # Matmul baut zusätzlich die Kachelgrößen-Reihe (Stufe 4 und CUDA)
  sycl_ziele=""; cuda_ziele="cuda"
  if [ "$b" = "Matmul" ]; then sycl_ziele="all kacheln"; cuda_ziele="cuda cuda-kacheln"; fi

  echo "=== $b: SYCL ==="
  # shellcheck disable=SC2086
  if ! make -C "$b" $sycl_ziele CXX="$SYCL_CXX" SYCL_CXX="$SYCL_CXX" \
                    SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 \
                    EXTRA="--cuda-path=${CUDA_HOME:-/usr}"; then
    echo "✗ $b: SYCL-Build fehlgeschlagen"; fehler=1
  fi
  if [ -n "${CUDA_HOME:-}" ]; then
    echo "=== $b: CUDA ==="
    # shellcheck disable=SC2086
    if ! make -C "$b" $cuda_ziele ARCH="$ARCHE"; then
      echo "✗ $b: CUDA-Build fehlgeschlagen"; fehler=1
    fi
  fi
  mkdir -p "$b/ergebnisse/dachs"
done
mkdir -p ergebnisse_dachs   # Protokolle der Smoke-Tests und AMD-Jobs

echo
echo "=== Gebaute Programme ==="
for b in $BEISPIELE; do
  [ -d "$b/bin" ] && { echo "$b:"; ls "$b/bin" | sed 's/^/  /'; }
done

echo
if [ "$fehler" -eq 0 ]; then
  echo "✓ Alles gebaut. Weiter mit dem Smoke-Test:"
  echo "    srun -p gpu1 --gres=gpu:1 --time=00:20:00 bash gemeinsam/dachs/smoke.sh"
else
  echo "✗ Mindestens ein Build ist fehlgeschlagen (siehe oben)."
fi
exit "$fehler"
