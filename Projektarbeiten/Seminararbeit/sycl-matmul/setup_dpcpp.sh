#!/usr/bin/env bash
# Holt DPC++ (Open-Source-Release von intel/llvm) und alles, was für die
# Messungen der Seminararbeit auf diesem Rechner nötig ist. Ohne sudo.
# Linux (auch WSL2), x86-64.
#
#   ./setup_dpcpp.sh
#   VERSION=v7.2.0 ONEAPI=2026.2 ./setup_dpcpp.sh
#
# Ablauf:
#   1. DPC++ laden und nach ~/sycl/dpcpp-<version> entpacken
#   2. Intels OpenCL-CPU-Laufzeit laden und nach ~/sycl/opencl-cpu-<oneapi>
#      entpacken. Das DPC++-Release enthält das OpenCL-Backend, aber keinen
#      OpenCL-Treiber für die CPU. Die Laufzeit läuft auch auf AMD-CPUs.
#   3. CUDA prüfen (für die NVIDIA-GPU: Treiber + CUDA Toolkit)
#   4. ~/sycl/env.sh schreiben, mit sycl-ls prüfen und Stufe 1 kurz testen
#
# Danach:  ./messen.sh   (lädt ~/sycl/env.sh selbst)

set -euo pipefail
cd "$(dirname "$0")"

VERSION="${VERSION:-v7.2.0}"          # intel/llvm-Release
ONEAPI="${ONEAPI:-2026.2}"            # dazu passende oneAPI-Version (siehe Release-Notes)
BASE="${BASE:-$HOME/sycl}"
PREFIX="${PREFIX:-$BASE/dpcpp-$VERSION}"
OCL_DIR="${OCL_DIR:-$BASE/opencl-cpu-$ONEAPI}"
ENV_FILE="${ENV_FILE:-$BASE/env.sh}"
URL="https://github.com/intel/llvm/releases/download/$VERSION/sycl_linux.tar.gz"
INTEL_REPO="https://apt.repos.intel.com/oneapi"

case "${1:-}" in -h|--help) sed -n '2,17p' "$0"; exit 0 ;; esac

info() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
ok()   { printf '\033[1;32m ok\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33mWarnung:\033[0m %s\n' "$*" >&2; }
fail() { printf '\033[1;31mFehler:\033[0m %s\n' "$*" >&2; exit 1; }

[ "$(uname -s)" = "Linux" ] || fail "Nur für Linux/WSL2. Unter Windows bitte WSL2 nutzen."
[ "$(uname -m)" = "x86_64" ] || fail "Das DPC++-Release gibt es nur für x86-64."
for cmd in curl tar gzip; do
  command -v "$cmd" >/dev/null || fail "$cmd fehlt (z. B. sudo apt install $cmd)"
done
command -v dpkg-deb >/dev/null || command -v ar >/dev/null \
  || fail "Zum Entpacken des Intel-Pakets wird dpkg-deb oder ar (binutils) benötigt"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$BASE"

# ---------------------------------------------------------------------------
# 1. DPC++
# ---------------------------------------------------------------------------
if [ -x "$PREFIX/bin/clang++" ]; then
  ok "DPC++ $VERSION ist schon installiert ($PREFIX)"
else
  info "Lade DPC++ $VERSION (ca. 250 MB)"
  curl -fL --progress-bar -o "$TMP/sycl_linux.tar.gz" "$URL" || fail "Download fehlgeschlagen: $URL"
  info "Entpacke nach $PREFIX"
  mkdir -p "$PREFIX"
  tar -xzf "$TMP/sycl_linux.tar.gz" -C "$PREFIX"
  rm -f "$TMP/sycl_linux.tar.gz"
  [ -x "$PREFIX/bin/clang++" ] || fail "Entpacken unvollständig, $PREFIX/bin/clang++ fehlt"
  ok "DPC++ installiert"
fi

# ---------------------------------------------------------------------------
# 2. OpenCL-CPU-Laufzeit (aus Intels Paket entpackt, nicht installiert)
# ---------------------------------------------------------------------------
if [ -f "$OCL_DIR/libintelocl.so" ]; then
  ok "OpenCL-CPU-Laufzeit ist schon vorhanden ($OCL_DIR)"
else
  info "Suche Intels Laufzeitpaket für oneAPI $ONEAPI"
  PAKET="intel-oneapi-compiler-shared-runtime-$ONEAPI"
  DATEI="$(curl -fsSL "$INTEL_REPO/dists/all/main/binary-amd64/Packages.gz" | gzip -dc \
           | awk -v p="pool/main/$PAKET-" '$1=="Filename:" && index($2, p)==1 {print $2}' | sort -V | tail -1)"
  [ -n "$DATEI" ] || fail "Paket $PAKET nicht im Intel-Repository gefunden (ONEAPI=... anpassen)"
  info "Lade $(basename "$DATEI") (ca. 100 MB)"
  curl -fL --progress-bar -o "$TMP/runtime.deb" "$INTEL_REPO/$DATEI" || fail "Download fehlgeschlagen"
  mkdir -p "$TMP/deb"
  if command -v dpkg-deb >/dev/null; then
    dpkg-deb -x "$TMP/runtime.deb" "$TMP/deb"
  else
    (cd "$TMP" && ar x runtime.deb && tar -xf data.tar.* -C deb)
  fi
  LIB="$(dirname "$(find "$TMP/deb" -name 'libintelocl.so' | head -1)")"
  [ -f "$LIB/libintelocl.so" ] || fail "libintelocl.so nicht im Paket gefunden"
  # Das Verzeichnis enthält nur Compiler-Laufzeit und OpenCL-CPU-Treiber,
  # kein libsycl – es kollidiert also nicht mit dem DPC++-Release
  mkdir -p "$OCL_DIR"
  cp -a "$LIB"/. "$OCL_DIR"/
  ok "OpenCL-CPU-Laufzeit nach $OCL_DIR entpackt"
fi

# ---------------------------------------------------------------------------
# 3. CUDA (nur wenn eine NVIDIA-GPU da ist)
# ---------------------------------------------------------------------------
CUDA_DIR=""
if command -v nvidia-smi >/dev/null && nvidia-smi -L >/dev/null 2>&1; then
  ok "NVIDIA-Treiber: $(nvidia-smi --query-gpu=name,driver_version --format=csv,noheader | head -1)"
  for d in "${CUDA_HOME:-}" "${CUDA_PATH:-}" /usr/local/cuda /usr/local/cuda-*; do
    if [ -n "$d" ] && [ -x "$d/bin/nvcc" ] && [ -d "$d/nvvm/libdevice" ]; then
      CUDA_DIR="$d"; break
    fi
  done
  if [ -n "$CUDA_DIR" ]; then
    ok "CUDA Toolkit: $CUDA_DIR ($("$CUDA_DIR/bin/nvcc" --version | grep -o 'release [0-9.]*'))"
  else
    warn "Kein CUDA Toolkit gefunden. DPC++ braucht es, um Kernel für NVIDIA zu übersetzen."
    echo "    Installation: https://developer.nvidia.com/cuda-downloads (Linux bzw. WSL-Ubuntu)"
    echo "    Für die RTX 5060 Ti (Blackwell) mindestens CUDA 12.8."
    echo "    Danach erneut: ./setup_dpcpp.sh  (oder CUDA_HOME=/pfad/zu/cuda ./setup_dpcpp.sh)"
  fi
else
  warn "Keine NVIDIA-GPU bzw. kein Treiber (nvidia-smi) gefunden – es wird nur die CPU gemessen."
fi

# ---------------------------------------------------------------------------
# 4. env.sh schreiben
# ---------------------------------------------------------------------------
if [ -f "$ENV_FILE" ] && ! grep -q "setup_dpcpp.sh" "$ENV_FILE"; then
  cp "$ENV_FILE" "$ENV_FILE.bak"
  warn "Vorhandene $ENV_FILE gesichert als $ENV_FILE.bak"
fi
{
  echo "# Erzeugt von setup_dpcpp.sh – DPC++ $VERSION, OpenCL-CPU-Laufzeit oneAPI $ONEAPI"
  echo "export DPCPP_HOME=\"$PREFIX\""
  echo "export PATH=\"\$DPCPP_HOME/bin:\$PATH\""
  echo "# DPC++ zuerst, die OpenCL-CPU-Laufzeit dahinter"
  echo "export LD_LIBRARY_PATH=\"\$DPCPP_HOME/lib:\${LD_LIBRARY_PATH:-}:$OCL_DIR\""
  echo "export OCL_ICD_FILENAMES=\"$OCL_DIR/libintelocl.so\""
  if [ -n "$CUDA_DIR" ]; then
    echo "export CUDA_HOME=\"$CUDA_DIR\""
    echo "export PATH=\"\$PATH:\$CUDA_HOME/bin\""
  fi
} > "$ENV_FILE"
ok "$ENV_FILE geschrieben"
# shellcheck disable=SC1090
source "$ENV_FILE"

# ---------------------------------------------------------------------------
# 5. Prüfen
# ---------------------------------------------------------------------------
echo
info "Sichtbare SYCL-Geräte (sycl-ls):"
LISTE="$(sycl-ls 2>/dev/null || true)"
echo "$LISTE" | sed 's/^/    /'
echo "$LISTE" | grep -q "opencl:cpu" || warn "Die CPU ist nicht als OpenCL-Gerät sichtbar."
if [ -n "$CUDA_DIR" ] && ! echo "$LISTE" | grep -q "cuda:gpu"; then
  warn "Die NVIDIA-GPU ist nicht sichtbar (Treiber zu alt? WSL: Windows-Treiber aktuell?)."
fi

if echo "$LISTE" | grep -q "cuda:gpu\|opencl:cpu"; then
  echo
  info "Kurztest: Stufe 1 bauen und auf jedem Gerät ausführen"
  TARGETS="spir64"
  echo "$LISTE" | grep -q "cuda:gpu" && [ -n "$CUDA_DIR" ] && TARGETS="nvptx64-nvidia-cuda,spir64"
  CUDA_FLAG=""; [ -n "$CUDA_DIR" ] && CUDA_FLAG="--cuda-path=$CUDA_DIR"
  if "$DPCPP_HOME/bin/clang++" -fsycl -O3 -fsycl-targets="$TARGETS" $CUDA_FLAG \
       -o "$TMP/stufe1" stufe1_usm_shared.cpp 2>"$TMP/build.log"; then
    for dev in cuda:gpu opencl:cpu; do
      echo "$LISTE" | grep -q "$dev" || continue
      if zeile=$(ONEAPI_DEVICE_SELECTOR=$dev "$TMP/stufe1" 256 1 2>&1); then
        ok "$dev: $zeile"
      else
        warn "$dev: Lauf fehlgeschlagen: $zeile"
      fi
    done
  else
    warn "Bauen fehlgeschlagen:"
    tail -15 "$TMP/build.log" >&2
  fi
fi

echo
info "Fertig. Messungen starten mit:  ./messen.sh"
echo "    In eigenen Shells vorher:   source $ENV_FILE"
