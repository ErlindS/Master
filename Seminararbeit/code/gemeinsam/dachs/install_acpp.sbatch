#!/bin/bash -l
# AdaptiveCpp (SYCL) mit HIP-Backend für die AMD MI300A bauen – ohne Admin-Rechte.
#
# Warum nicht DPC++? Die intel/llvm-Pakete (Release 7.2.0 und Nightly) enthalten
# nur den CUDA- und Level-Zero-Adapter, keinen HIP-Adapter (libur_adapter_hip.so).
#
# Läuft auf dem MI300A-Knoten, weil ROCm nur dort sicher vorhanden ist.
# Nicht direkt abschicken, sondern über den Wrapper (findet die Partition):
#
#   lokal:    curl -fL -o AdaptiveCpp-25.10.0.tar.gz \
#               https://github.com/AdaptiveCpp/AdaptiveCpp/archive/refs/tags/v25.10.0.tar.gz
#             scp AdaptiveCpp-25.10.0.tar.gz hk_seer1012@dachs-login.hs-esslingen.de:~
#   Cluster:  bash gemeinsam/dachs/amd.sh install      (im Ordner code)
#
# Ergebnis:   ~/sycl/acpp-25.10.0/ und ~/sycl/env_amd.sh
# Protokoll:  ergebnisse_dachs/install_acpp_<jobid>.out
#
# Einstellungen: ACPP_TARBALL, ACPP_PREFIX, LLVM_MODULE (z. B. compiler/llvm/19
#                erzwingen), ROCM_MODULE
#
#SBATCH --job-name=install-acpp
#SBATCH --partition=gpu_mi300a
#SBATCH --gres=gpu:mi300a:1
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=32G
#SBATCH --time=01:00:00
#SBATCH --output=ergebnisse_dachs/install_acpp_%j.out

set -uo pipefail

ACPP_VERSION="25.10.0"
ACPP_TARBALL="${ACPP_TARBALL:-$HOME/AdaptiveCpp-$ACPP_VERSION.tar.gz}"
ACPP_PREFIX="${ACPP_PREFIX:-$HOME/sycl/acpp-$ACPP_VERSION}"
JOBS="${SLURM_CPUS_PER_TASK:-8}"
# AdaptiveCpp 25.10 unterstützt offiziell LLVM 15–20
LLVM_MAX=20

info() { echo; echo "==> $*"; }
fail() { echo; echo "FEHLER: $*" >&2; exit 1; }

info "Knoten: $(hostname)"
[ -f "$ACPP_TARBALL" ] || fail "$ACPP_TARBALL fehlt (siehe Kopf des Skripts: lokal laden, per scp kopieren)."

# ---------------------------------------------------------------------------
# 1. ROCm finden
# ---------------------------------------------------------------------------
info "Suche ROCm"
ROCM_MODULE_GELADEN=""
rocm_aus_pfad() {
  for p in "${ROCM_PATH:-}" /opt/rocm $(ls -d /opt/rocm-* 2>/dev/null | sort -V -r); do
    [ -n "$p" ] && [ -x "$p/bin/hipcc" ] && { echo "$p"; return 0; }
  done
  return 1
}
ROCM="$(rocm_aus_pfad || true)"
if [ -z "$ROCM" ]; then
  mod="${ROCM_MODULE:-$(module -t avail 2>&1 | grep -iE '(^|/)rocm' | sort -V | tail -1)}"
  if [ -n "$mod" ]; then
    echo "Lade Modul $mod"
    module load "$mod" && ROCM_MODULE_GELADEN="$mod"
    ROCM="$(rocm_aus_pfad || true)"
    [ -z "$ROCM" ] && command -v hipcc >/dev/null && ROCM="$(dirname "$(dirname "$(readlink -f "$(command -v hipcc)")")")"
  fi
fi
[ -n "$ROCM" ] || fail "Kein ROCm gefunden (weder ROCM_PATH, /opt/rocm* noch ein rocm-Modul).
  Ohne ROCm lässt sich die MI300A nicht ansprechen. Beim DACHS-Support nach ROCm auf dem MI300A-Knoten fragen."
export ROCM_PATH="$ROCM"
echo "ROCm: $ROCM  ($(cat "$ROCM/.info/version" 2>/dev/null || echo 'Version unbekannt'))"

ROCM_CLANG=""
for c in "$ROCM/llvm/bin/clang++" "$ROCM/lib/llvm/bin/clang++"; do [ -x "$c" ] && { ROCM_CLANG="$c"; break; }; done
ROCM_LLVM_MAJOR=99
if [ -n "$ROCM_CLANG" ]; then
  ROCM_LLVM_MAJOR=$("$ROCM_CLANG" --version | grep -oE 'clang version [0-9]+' | grep -oE '[0-9]+$')
  echo "ROCm-LLVM: $ROCM_LLVM_MAJOR ($ROCM_CLANG)"
fi
# generic-JIT: LLVM von AdaptiveCpp muss <= ROCm-LLVM sein
CAP=$(( ROCM_LLVM_MAJOR < LLVM_MAX ? ROCM_LLVM_MAJOR : LLVM_MAX ))
echo "Erlaubte LLVM-Hauptversion: 15 … $CAP"

# GPU sichtbar?
"$ROCM/bin/rocminfo" 2>/dev/null | grep -m2 -E 'Marketing Name:.*(MI|Instinct)|Name: *gfx9' || echo "Warnung: rocminfo zeigt keine MI300A."

# ---------------------------------------------------------------------------
# 2. Passendes LLVM finden (Module compiler/llvm/*, sonst ROCm-LLVM)
# ---------------------------------------------------------------------------
info "Suche LLVM"
LLVM_PREFIX=""; LLVM_MODULE_GELADEN=""; LLD=""
pruefe_llvm() {   # pruefe_llvm <prefix> <max_major>  -> 0, wenn brauchbar
  local pre="$1" max="$2" cfg="$1/bin/llvm-config" major libdir cm
  [ -x "$cfg" ] || { echo "   - kein llvm-config"; return 1; }
  major=$("$cfg" --version | cut -d. -f1)
  [ "$major" -ge 15 ] && [ "$major" -le "$max" ] || { echo "   - LLVM $major außerhalb 15…$max"; return 1; }
  cm=$("$cfg" --cmakedir)
  [ -f "$cm/LLVMConfig.cmake" ] || { echo "   - LLVMConfig.cmake fehlt (keine Entwicklerdateien)"; return 1; }
  ls "$pre"/lib*/cmake/clang/ClangConfig.cmake >/dev/null 2>&1 || { echo "   - ClangConfig.cmake fehlt"; return 1; }
  "$cfg" --targets-built | grep -qw AMDGPU || { echo "   - ohne AMDGPU-Backend gebaut"; return 1; }
  libdir=$("$cfg" --libdir)
  ls "$libdir"/libLLVM*.so* >/dev/null 2>&1 || { echo "   - libLLVM.so fehlt (statisch gebaut)"; return 1; }
  [ -f "$("$cfg" --includedir)/clang/AST/ASTContext.h" ] || { echo "   - clang-Header fehlen"; return 1; }
  for t in llc opt; do [ -x "$pre/bin/$t" ] || { echo "   - $t fehlt"; return 1; }; done
  # ld.lld ist Pflicht; fehlt es, darf es aus ROCm kommen
  LLD=""
  for l in "$pre/bin/ld.lld" "$ROCM/llvm/bin/ld.lld" "$ROCM/lib/llvm/bin/ld.lld"; do
    [ -x "$l" ] && { LLD="$l"; break; }
  done
  [ -n "$LLD" ] || { echo "   - ld.lld fehlt (auch nicht in ROCm)"; return 1; }
  echo "   + LLVM $major brauchbar (ld.lld: $LLD)"
  return 0
}

if [ -n "${LLVM_MODULE:-}" ]; then
  KANDIDATEN="$LLVM_MODULE"
else
  KANDIDATEN=$(module -t avail compiler/llvm 2>&1 | grep -E '^compiler/llvm/[0-9]' | sort -V -r)
fi
for mod in $KANDIDATEN; do
  echo " Modul $mod"
  module unload compiler/llvm >/dev/null 2>&1
  module load "$mod" >/dev/null 2>&1 || { echo "   - lässt sich nicht laden"; continue; }
  cl="$(command -v clang++ || true)"
  [ -n "$cl" ] || { echo "   - kein clang++"; continue; }
  pre="$(dirname "$(dirname "$(readlink -f "$cl")")")"
  if pruefe_llvm "$pre" "$CAP"; then LLVM_PREFIX="$pre"; LLVM_MODULE_GELADEN="$mod"; break; fi
done
if [ -z "$LLVM_PREFIX" ]; then
  module unload compiler/llvm >/dev/null 2>&1
  if [ -n "$ROCM_CLANG" ]; then
    pre="$(dirname "$(dirname "$ROCM_CLANG")")"
    echo " ROCm-LLVM $pre (laut AdaptiveCpp-Doku 'weniger getestet', aber oft brauchbar)"
    pruefe_llvm "$pre" 99 && LLVM_PREFIX="$pre"
  fi
fi
[ -n "$LLVM_PREFIX" ] || fail "Kein brauchbares LLVM gefunden (siehe Gründe oben).
  Bitte die Ausgabe von 'module avail compiler/llvm' und dieses Protokoll schicken."
CFG="$LLVM_PREFIX/bin/llvm-config"
echo "Verwende LLVM $("$CFG" --version) aus $LLVM_PREFIX ${LLVM_MODULE_GELADEN:+(Modul $LLVM_MODULE_GELADEN)}"

# Host-Compiler: clang, wenn libomp dabei ist (CPU-Backend braucht OpenMP), sonst gcc
if ls "$LLVM_PREFIX"/lib*/libomp.so* >/dev/null 2>&1 || ls "$LLVM_PREFIX"/lib/*/libomp.so* >/dev/null 2>&1; then
  CC_="$LLVM_PREFIX/bin/clang"; CXX_="$LLVM_PREFIX/bin/clang++"
else
  CC_="$(command -v gcc)"; CXX_="$(command -v g++)"
  echo "Hinweis: libomp fehlt im LLVM, AdaptiveCpp wird mit $CXX_ gebaut."
fi

# ---------------------------------------------------------------------------
# 3. CMake / Python
# ---------------------------------------------------------------------------
if ! command -v cmake >/dev/null; then
  mod=$(module -t avail 2>&1 | grep -iE '(^|/)cmake' | sort -V | tail -1)
  [ -n "$mod" ] && module load "$mod"
fi
command -v cmake   >/dev/null || fail "cmake fehlt (auch kein cmake-Modul)."
command -v python3 >/dev/null || fail "python3 fehlt."
echo "cmake: $(cmake --version | head -1)   python3: $(python3 --version)"

# ---------------------------------------------------------------------------
# 4. Bauen
# ---------------------------------------------------------------------------
WORK="${TMPDIR:-/tmp}/acpp_build_$$"
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT
info "Entpacke nach $WORK"
tar -xzf "$ACPP_TARBALL" -C "$WORK"
SRC="$(ls -d "$WORK"/AdaptiveCpp-* | head -1)"

info "cmake"
# OpenCL/Level Zero aus: brauchen wir nicht, und sie laden beim Bauen Code von
# GitHub nach (der Knoten hat kein Internet). CUDA aus: kein NVIDIA auf dem Knoten.
cmake -S "$SRC" -B "$WORK/build" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$ACPP_PREFIX" \
  -DCMAKE_C_COMPILER="$CC_" -DCMAKE_CXX_COMPILER="$CXX_" \
  -DLLVM_DIR="$("$CFG" --cmakedir)" \
  -DCLANG_EXECUTABLE_PATH="$LLVM_PREFIX/bin/clang++" \
  -DACPP_LLD_PATH="$LLD" \
  -DROCM_PATH="$ROCM" \
  -DWITH_ROCM_BACKEND=ON \
  -DWITH_CUDA_BACKEND=OFF \
  -DWITH_OPENCL_BACKEND=OFF \
  -DWITH_LEVEL_ZERO_BACKEND=OFF \
  || fail "cmake fehlgeschlagen (siehe oben)."

info "Bauen mit $JOBS Kernen"
cmake --build "$WORK/build" -j "$JOBS" || fail "Bauen fehlgeschlagen."
rm -rf "$ACPP_PREFIX"
cmake --install "$WORK/build" || fail "Installieren fehlgeschlagen."

# ---------------------------------------------------------------------------
# 5. Umgebung für Jobs
# ---------------------------------------------------------------------------
cat > "$HOME/sycl/env_amd.sh" <<EOF
# AdaptiveCpp $ACPP_VERSION für AMD MI300A (erzeugt von install_acpp.sbatch, Knoten $(hostname))
${ROCM_MODULE_GELADEN:+module load $ROCM_MODULE_GELADEN}
${LLVM_MODULE_GELADEN:+module load $LLVM_MODULE_GELADEN}
export ROCM_PATH="$ROCM"
export ACPP_HOME="$ACPP_PREFIX"
export PATH="\$ACPP_HOME/bin:\$ROCM_PATH/bin:\$PATH"
export LD_LIBRARY_PATH="\$ACPP_HOME/lib:\$ROCM_PATH/lib:\${LD_LIBRARY_PATH:-}"
# generic = ein Durchlauf, Kernel werden zur Laufzeit für gfx942 übersetzt (wie PTX bei NVIDIA).
# Alternative ohne JIT:  export ACPP_TARGETS=hip:gfx942
export ACPP_TARGETS="\${ACPP_TARGETS:-generic}"
EOF

info "Test"
# shellcheck disable=SC1091
source "$HOME/sycl/env_amd.sh"
acpp --acpp-version 2>&1 | head -20
echo
acpp-info -l 2>&1
cat > "$WORK/test.cpp" <<'EOF'
#include <sycl/sycl.hpp>
#include <iostream>
int main() {
  sycl::queue q;
  int* x = sycl::malloc_shared<int>(1024, q);
  q.parallel_for(1024, [=](sycl::id<1> i) { x[i] = 2 * int(i[0]); }).wait();
  bool ok = true;
  for (int i = 0; i < 1024; ++i) ok &= (x[i] == 2 * i);
  std::cout << q.get_device().get_info<sycl::info::device::name>() << ": " << (ok ? "OK" : "FALSCH") << "\n";
  sycl::free(x, q);
  return ok ? 0 : 1;
}
EOF
acpp -O2 -o "$WORK/test" "$WORK/test.cpp" && ACPP_VISIBILITY_MASK=hip "$WORK/test" \
  || fail "Testprogramm läuft nicht auf der GPU."

echo
echo "Fertig. AdaptiveCpp liegt in $ACPP_PREFIX, Umgebung: source ~/sycl/env_amd.sh"
echo "Weiter (im Ordner code):  bash gemeinsam/dachs/amd.sh messen"
