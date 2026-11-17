#!/usr/bin/env bash
# dachs — kurzer Befehl für den DACHS-Cluster (HS Esslingen / bwHPC).
#
#   dachs                  Shell auf DACHS, direkt im gleichnamigen Projektordner
#   dachs <befehl …>       Befehl dort ausführen, z. B.  dachs squeue --me
#   dachs push             aktuellen Ordner nach ~/<ordnername> hochladen
#   dachs pull <pfad>      ~/<ordnername>/<pfad> hierher holen
#   dachs off              Verbindung trennen
#
# OTP + Passwort fragt DACHS nur EINMAL ab; danach läuft alles über die offene
# Verbindung (Standard 8h, änderbar mit DACHS_PERSIST=4h).
# Einmalig installieren:  bash dachs/verbinden.sh install   (legt ~/.local/bin/dachs an)

set -euo pipefail

USR="${DACHS_USER:-hk_seer1012}"
HOST="${DACHS_HOST:-dachs-login.hs-esslingen.de}"
PERSIST="${DACHS_PERSIST:-8h}"
KEY="${DACHS_KEY:-$HOME/.ssh/id_ed25519}"
mkdir -p -m 700 "$HOME/.ssh/sockets"

OPTS=(-o ControlMaster=auto -o "ControlPath=$HOME/.ssh/sockets/dachs" -o "ControlPersist=$PERSIST"
      -o ServerAliveInterval=60 -o LogLevel=ERROR)
[ -f "$KEY" ] && OPTS+=(-i "$KEY")
T="$USR@$HOST"
PROJ="$(basename "$PWD")"   # Projektordner = Name des aktuellen Ordners

verbinden() {
  ssh "${OPTS[@]}" -O check "$T" 2>/dev/null && return
  if ! timeout 8 bash -c "exec 3<>/dev/tcp/$HOST/22" 2>/dev/null; then
    echo "✗ $HOST nicht erreichbar (Port 22). Ist die HKA-VPN verbunden?" >&2
    echo "  → FortiClient öffnen (Tray-Symbol oder: /opt/forticlient/fortitray), HKA-VPN verbinden, dann nochmal: dachs" >&2
    exit 1
  fi
  echo "→ Anmeldung bei DACHS (OTP, dann Passwort):"
  ssh "${OPTS[@]}" -fN "$T"
}

case "${1:-}" in
  install)
    ln -sf "$(realpath "$0")" "$HOME/.local/bin/dachs"
    echo "Fertig. Ab jetzt einfach:  dachs" ;;
  off)
    ssh "${OPTS[@]}" -O exit "$T" 2>/dev/null || true ;;
  push)
    verbinden
    rsync -az --info=progress2 --exclude .git --exclude 'build*/' -e "ssh ${OPTS[*]}" ./ "$T:$PROJ/" ;;
  pull)
    verbinden
    rsync -az --info=progress2 -e "ssh ${OPTS[*]}" "$T:$PROJ/${2:?Was holen? z. B.  dachs pull ergebnisse_dachs}" "${3:-.}" ;;
  "")
    verbinden
    ssh "${OPTS[@]}" -t "$T" "cd '$PROJ' 2>/dev/null; exec \$SHELL -l" ;;
  -h|--help)
    sed -n '2,13p' "$0" | sed 's/^# \{0,1\}//' ;;
  *)
    verbinden
    ssh "${OPTS[@]}" "$T" "cd '$PROJ' 2>/dev/null; $*" ;;
esac
