#!/bin/bash -l
# Prüft, was auf dem MI300A-Knoten für SYCL auf AMD zur Verfügung steht.
# Dauert unter einer Minute. Im Ordner sycl-matmul abschicken:
#
#   sbatch dachs/amd_check.sbatch
#   cat ergebnisse_dachs/amd_check_<jobid>.out
#
#SBATCH --job-name=amd-check
#SBATCH --partition=gpu_mi300a
#SBATCH --gres=gpu:mi300a:1
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --mem=8G
#SBATCH --time=00:05:00
#SBATCH --output=ergebnisse_dachs/amd_check_%j.out

abschnitt() { echo; echo "== $*"; }

abschnitt "Knoten / Betriebssystem"
hostname; grep PRETTY_NAME /etc/os-release; uname -r

abschnitt "ROCm-Installationen"
ls -d /opt/rocm* 2>&1
echo "ROCM_PATH=${ROCM_PATH:-}"
cat /opt/rocm/.info/version 2>/dev/null

abschnitt "Module mit rocm/hip/amd/llvm/compiler (auf dem Knoten)"
module avail 2>&1 | grep -iE 'rocm|hip|amd|llvm|clang|compiler|cmake|apptainer|singularity' || echo "(keine Treffer)"

abschnitt "GPU"
for c in rocm-smi rocminfo amd-smi; do
  p=$(command -v $c || ls /opt/rocm*/bin/$c 2>/dev/null | head -1)
  echo "$c: ${p:-fehlt}"
done
r=$(command -v rocminfo || ls /opt/rocm*/bin/rocminfo 2>/dev/null | head -1)
[ -n "$r" ] && "$r" 2>&1 | grep -m4 -E 'Marketing Name|Name: *gfx'
ls -l /dev/kfd /dev/dri 2>&1 | head -5

abschnitt "Compiler und Werkzeuge"
for c in hipcc amdclang++ clang++ cmake gcc g++ python3 git curl; do
  p=$(command -v $c || ls /opt/rocm*/bin/$c /opt/rocm*/llvm/bin/$c 2>/dev/null | head -1)
  v=""; [ -n "$p" ] && v=$("$p" --version 2>&1 | head -1)
  printf '%-11s %s  %s\n' "$c:" "${p:-fehlt}" "$v"
done
ls /opt/rocm*/llvm/lib/cmake 2>/dev/null | head -5

abschnitt "Container"
for c in apptainer singularity; do echo "$c: $(command -v $c || echo fehlt)"; done

abschnitt "Internet vom Knoten"
curl -sS -m 8 -o /dev/null -w 'github.com: HTTP %{http_code}\n' https://github.com 2>&1

abschnitt "Vorhandenes DPC++"
source "$HOME/sycl/env.sh" 2>/dev/null
ls "${DPCPP_HOME:-/fehlt}/lib" 2>/dev/null | grep -E '^libur_adapter_.*\.so$'
sycl-ls 2>&1 | head -5
