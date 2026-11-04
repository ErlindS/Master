#!/usr/bin/env bash
# DPC++ (Open-Source-Release von intel/llvm) ohne Admin-Rechte installieren.
# Auf dem LOGIN-Knoten ausführen (keine GPU nötig).
#
# Der Cluster hat kein Internet. Das Archiv deshalb lokal laden und kopieren:
#   lokal:    curl -fLO https://github.com/intel/llvm/releases/download/v7.2.0/sycl_linux.tar.gz
#             scp sycl_linux.tar.gz hk_seer1012@dachs-login.hs-esslingen.de:~
#   Cluster:  bash dachs/install_dpcpp.sh            (sucht ~/sycl_linux.tar.gz)
#             TARBALL=/pfad/zur/datei bash dachs/install_dpcpp.sh
#
# Danach in jeder Shell / jedem Job:  source ~/sycl/env.sh

set -euo pipefail

VERSION="${VERSION:-v7.2.0}"
PREFIX="${PREFIX:-$HOME/sycl/dpcpp-$VERSION}"
TARBALL="${TARBALL:-$HOME/sycl_linux.tar.gz}"
URL="https://github.com/intel/llvm/releases/download/$VERSION/sycl_linux.tar.gz"

if [ ! -x "$PREFIX/bin/clang++" ]; then
  if [ ! -f "$TARBALL" ]; then
    echo "Archiv $TARBALL fehlt. Lokal laden und kopieren:" >&2
    echo "  curl -fLO $URL" >&2
    echo "  scp sycl_linux.tar.gz hk_seer1012@dachs-login.hs-esslingen.de:~" >&2
    exit 1
  fi
  mkdir -p "$PREFIX"
  echo "==> Entpacke $TARBALL nach $PREFIX"
  tar -xzf "$TARBALL" -C "$PREFIX"
fi

cat > "$HOME/sycl/env.sh" <<EOF
# DPC++ $VERSION + CUDA-Modul
module load devel/cuda/12.8.1
export DPCPP_HOME="$PREFIX"
export PATH="\$DPCPP_HOME/bin:\$PATH"
export LD_LIBRARY_PATH="\$DPCPP_HOME/lib:\${LD_LIBRARY_PATH:-}"
EOF

echo "==> Enthaltene Backends (Unified-Runtime-Adapter):"
ls "$PREFIX/lib" | grep -E '^libur_adapter_.*\.so$' || echo "   keine gefunden?!"
echo
echo "Für NVIDIA wird libur_adapter_cuda.so gebraucht, für AMD libur_adapter_hip.so."
echo "Fertig. Weiter mit:  source ~/sycl/env.sh && bash dachs/build.sh"
