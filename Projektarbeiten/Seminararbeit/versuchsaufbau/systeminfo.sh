#!/usr/bin/env bash
# Hält Hardware, Treiber und Compilerversionen fest (Reproduzierbarkeit).
# Fehlende Werkzeuge werden übersprungen.
#
#   ./systeminfo.sh > messdaten/system.txt

abschnitt() {
  echo
  echo "== $1"
  shift
  "$@" 2>&1 || echo "(nicht verfügbar)"
}

echo "Zeitpunkt: $(date -Iseconds)"
abschnitt "System"            uname -a
abschnitt "CPU"               bash -c "lscpu | grep -E 'Model name|^CPU\(s\)|Thread|Core|Socket|MHz|L[123]'"
abschnitt "CPU-Governor"      cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
abschnitt "Speicher"          free -h
abschnitt "GPU"               nvidia-smi --query-gpu=name,driver_version,clocks.max.sm,clocks.max.mem,memory.total,power.limit --format=csv
abschnitt "CUDA"              nvcc --version
abschnitt "DPC++"             bash -c "${DPCPP_CXX:-icpx} --version"
abschnitt "SYCL-Geräte (DPC++)" sycl-ls
abschnitt "AdaptiveCpp"       acpp --version
abschnitt "SYCL-Geräte (AdaptiveCpp)" acpp-info -l
abschnitt "GCC"               g++ --version
abschnitt "OpenBLAS"          bash -c "ldconfig -p 2>/dev/null | grep -i openblas || ls /usr/lib/x86_64-linux-gnu | grep -i openblas"
abschnitt "Git-Stand"         git -C "$(dirname "$0")" log -1 --format='%H %cd'
abschnitt "Uncommitted"       git -C "$(dirname "$0")" status --short .
