#!/usr/bin/env bash
# Richtet alles Nötige ein und startet den Benchmark – unabhängig davon, was
# auf dem Rechner schon installiert ist. Läuft unter Linux (auch WSL2) und
# macOS; unter Windows stattdessen setup.ps1 verwenden.
#
#   ./setup.sh              interaktiv (fragt vor jeder Installation)
#   ./setup.sh --yes        ohne Rückfragen
#   ./setup.sh --cpu        nur CPU messen, GPU ignorieren
#   ./setup.sh --rebuild    Image neu bauen (z. B. nach Treiber-Update)
#
#   SIZES="512 1024" REPS=3 ./setup.sh   (schneller Probelauf)
#
# Ablauf:
#   1. Docker prüfen und bei Bedarf installieren bzw. starten
#   2. NVIDIA-GPU erkennen; unter Linux bei Bedarf das NVIDIA Container
#      Toolkit installieren, damit Docker die GPU nutzen kann
#   3. Image mit LLVM + AdaptiveCpp bauen (einmalig, ca. 15–30 Minuten)
#   4. Benchmark im Container ausführen; Ergebnisse landen in
#      ergebnisse/<datum>_<cpu|cuda>/

set -euo pipefail
cd "$(dirname "$0")"

CUDA_BASE_IMAGE="${CUDA_BASE_IMAGE:-nvidia/cuda:12.9.1-devel-ubuntu24.04}"
YES=0
CPU_ONLY=0
REBUILD=0
for arg in "$@"; do
  case "$arg" in
    -y|--yes) YES=1 ;;
    --cpu) CPU_ONLY=1 ;;
    --rebuild) REBUILD=1 ;;
    -h|--help) sed -n '2,19p' "$0"; exit 0 ;;
    *) echo "Unbekannte Option: $arg (siehe --help)" >&2; exit 2 ;;
  esac
done

info() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33mWarnung:\033[0m %s\n' "$*" >&2; }
fail() { printf '\033[1;31mFehler:\033[0m %s\n' "$*" >&2; exit 1; }
frage() {
  [ "$YES" = 1 ] && return 0
  local antwort
  read -r -p "$1 [j/N] " antwort
  [[ "$antwort" =~ ^[jJyY] ]]
}

OS=$(uname -s)
case "$OS" in
  Linux|Darwin) ;;
  MINGW*|MSYS*|CYGWIN*) fail "Unter Windows bitte setup.ps1 in PowerShell ausführen." ;;
  *) fail "Nicht unterstütztes Betriebssystem: $OS" ;;
esac

SUDO=""
if [ "$(id -u)" -ne 0 ]; then
  command -v sudo >/dev/null 2>&1 && SUDO=sudo
fi

# --------------------------------------------------------------------------
# 1. Docker
# --------------------------------------------------------------------------
installiere_docker() {
  if [ "$OS" = Darwin ]; then
    command -v brew >/dev/null 2>&1 ||
      fail "Docker fehlt. Bitte Docker Desktop installieren: https://www.docker.com/products/docker-desktop/"
    brew install --cask docker
    open -a Docker
    return
  fi
  if command -v apt-get >/dev/null 2>&1; then
    $SUDO apt-get update && $SUDO apt-get install -y docker.io
  elif command -v dnf >/dev/null 2>&1; then
    $SUDO dnf install -y moby-engine || $SUDO dnf install -y docker
  elif command -v pacman >/dev/null 2>&1; then
    $SUDO pacman -Sy --noconfirm docker
  elif command -v zypper >/dev/null 2>&1; then
    $SUDO zypper install -y docker
  else
    fail "Kein bekannter Paketmanager. Docker bitte manuell installieren: https://docs.docker.com/engine/install/"
  fi
  $SUDO systemctl enable --now docker 2>/dev/null || $SUDO service docker start || true
}

if ! command -v docker >/dev/null 2>&1; then
  frage "Docker ist nicht installiert. Jetzt installieren?" || fail "Abgebrochen."
  installiere_docker
fi

# Läuft der Docker-Dienst, und brauchen wir sudo?
DOCKER=(docker)
if ! docker info >/dev/null 2>&1; then
  if [ "$OS" = Darwin ]; then
    open -a Docker 2>/dev/null || true
  else
    $SUDO systemctl start docker 2>/dev/null || $SUDO service docker start 2>/dev/null || true
  fi
  info "Warte auf den Docker-Dienst ..."
  for _ in $(seq 1 60); do
    docker info >/dev/null 2>&1 && break
    if [ "$OS" = Linux ] && [ -n "$SUDO" ] && $SUDO docker info >/dev/null 2>&1; then
      DOCKER=($SUDO docker)
      warn "Docker läuft nur mit sudo. Dauerhaft ohne sudo: sudo usermod -aG docker \$USER (danach neu anmelden)."
      break
    fi
    sleep 2
  done
  "${DOCKER[@]}" info >/dev/null 2>&1 || fail "Docker läuft nicht. Bitte Docker starten und das Skript erneut ausführen."
fi
info "Docker: $("${DOCKER[@]}" version --format '{{.Server.Version}}')"

# --------------------------------------------------------------------------
# 2. NVIDIA-GPU
# --------------------------------------------------------------------------
installiere_nvidia_toolkit() {
  command -v curl >/dev/null 2>&1 || fail "curl fehlt (für das NVIDIA Container Toolkit benötigt)."
  local repo=https://nvidia.github.io/libnvidia-container
  if command -v apt-get >/dev/null 2>&1; then
    curl -fsSL "$repo/gpgkey" |
      $SUDO gpg --dearmor --yes -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg
    curl -fsSL "$repo/stable/deb/nvidia-container-toolkit.list" |
      sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' |
      $SUDO tee /etc/apt/sources.list.d/nvidia-container-toolkit.list >/dev/null
    $SUDO apt-get update && $SUDO apt-get install -y nvidia-container-toolkit
  elif command -v dnf >/dev/null 2>&1; then
    curl -fsSL "$repo/stable/rpm/nvidia-container-toolkit.repo" |
      $SUDO tee /etc/yum.repos.d/nvidia-container-toolkit.repo >/dev/null
    $SUDO dnf install -y nvidia-container-toolkit
  elif command -v zypper >/dev/null 2>&1; then
    $SUDO zypper ar -f "$repo/stable/rpm/nvidia-container-toolkit.repo" || true
    $SUDO zypper --gpg-auto-import-keys install -y nvidia-container-toolkit
  elif command -v pacman >/dev/null 2>&1; then
    $SUDO pacman -Sy --noconfirm nvidia-container-toolkit
  else
    warn "Kein bekannter Paketmanager für das NVIDIA Container Toolkit."
    return 1
  fi
  $SUDO nvidia-ctk runtime configure --runtime=docker
  $SUDO systemctl restart docker 2>/dev/null || $SUDO service docker restart || true
}

gpu_im_container() {
  "${DOCKER[@]}" run --rm --gpus all ubuntu:24.04 true >/dev/null 2>&1
}

GPU=0
if [ "$CPU_ONLY" = 0 ] && command -v nvidia-smi >/dev/null 2>&1 && nvidia-smi -L >/dev/null 2>&1; then
  info "NVIDIA-GPU gefunden: $(nvidia-smi -L | head -1)"
  if ! gpu_im_container && [ "$OS" = Linux ]; then
    warn "Docker kann die GPU noch nicht nutzen (NVIDIA Container Toolkit fehlt)."
    if frage "NVIDIA Container Toolkit installieren?"; then
      installiere_nvidia_toolkit || true
    fi
  fi
  if gpu_im_container; then
    GPU=1
  else
    warn "GPU im Container nicht nutzbar – es wird nur die CPU gemessen."
  fi
else
  info "Keine nutzbare NVIDIA-GPU – es wird nur die CPU gemessen."
fi

# --------------------------------------------------------------------------
# 3. Image bauen
# --------------------------------------------------------------------------
if [ "$GPU" = 1 ]; then
  TAG=sycl-matmul:cuda
  BUILD_ARGS=(--build-arg "BASE_IMAGE=$CUDA_BASE_IMAGE" --build-arg WITH_CUDA=ON)
else
  TAG=sycl-matmul:cpu
  BUILD_ARGS=(--build-arg WITH_CUDA=OFF)
fi

if [ "$REBUILD" = 1 ] || ! "${DOCKER[@]}" image inspect "$TAG" >/dev/null 2>&1; then
  info "Baue Image $TAG (beim ersten Mal ca. 15–30 Minuten) ..."
  "${DOCKER[@]}" build "${BUILD_ARGS[@]}" -t "$TAG" -f docker/Dockerfile docker/
fi

# --------------------------------------------------------------------------
# 4. Benchmark ausführen
# --------------------------------------------------------------------------
OUTDIR="$PWD/ergebnisse/$(date +%Y%m%d_%H%M)_${TAG#sycl-matmul:}"
mkdir -p "$OUTDIR"

RUN_ARGS=(--rm -v "$PWD:/work/sycl-matmul:ro" -v "$OUTDIR:/ergebnisse")
for var in SIZES REPS TILES SEQ_MAX; do
  [ -n "${!var:-}" ] && RUN_ARGS+=(-e "$var")
done
[ "$GPU" = 1 ] && RUN_ARGS+=(--gpus all)
# Unter Linux als eigener Benutzer laufen, damit die Ergebnisse dir gehören
[ "$OS" = Linux ] && RUN_ARGS+=(--user "$(id -u):$(id -g)")

info "Starte Benchmark (Ergebnisse: ${OUTDIR#"$PWD"/})"
"${DOCKER[@]}" run "${RUN_ARGS[@]}" "$TAG" bash /work/sycl-matmul/docker/run_in_container.sh
info "Fertig. Ergebnisse in ${OUTDIR#"$PWD"/}/"
