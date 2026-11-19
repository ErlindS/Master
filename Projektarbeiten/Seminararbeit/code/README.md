# Code zur Seminararbeit „SYCL – Portable heterogene Programmierung mit C++“

| Ordner | Inhalt |
|---|---|
| `Matmul/` | Matrixmultiplikation, Stufe 0–5 (`StufeN/{Normal,Sycl,Cuda}`), cuBLAS-Referenz |
| `Stencil/` | 2D-Stencil, Varianten `1_2D` … `7_Geraet` (`4`–`7` sind die Gegenstücke der Matmul-Stufen 1, 2b, 3 und 5) |
| `Reduktion/` | Reduktion/Atomics, Varianten `1_Atomar` … `7_Geraet` (`5`–`7` sind die Gegenstücke der Matmul-Stufen 1, 2b und 5) |
| `gemeinsam/` | Messrahmen für alle Beispiele (`common.hpp`, `probleme.hpp`, `sycl_rahmen.hpp`, `cuda_rahmen.cuh`) und DACHS-Werkzeuge (`dachs/`) |
| `versuchsaufbau/` | erweiterter Versuchsaufbau (Hypothesen H1–H5, Triade, Autotuning) |
| `GetDevices/` | listet die sichtbaren SYCL-Geräte |
| `sycl-acpp-example/` | AdaptiveCpp-Beispielprojekt |

Jedes Beispiel hat ein eigenes `Makefile` (baut nach `bin/`), ein README mit
Befehlen und Ergebnissen, einen Ordner `gemeinsam/` mit Mess- und DACHS-Skripten
und einen Ordner `ergebnisse/` mit den Rohdaten.

## DACHS

```bash
# einmalig: Kurzbefehl "dachs" installieren
bash gemeinsam/dachs/verbinden.sh install

# im Ordner code hochladen -> ~/code auf DACHS
dachs push
dachs                     # Shell auf DACHS in ~/code
```

DPC++ einmalig installieren: `bash gemeinsam/dachs/install_dpcpp.sh` (siehe
Kopf des Skripts). Danach je Beispiel die Befehle aus dem jeweiligen README.

### AMD MI300A (AdaptiveCpp)

Die DPC++-Pakete von intel/llvm (Release 7.2.0 und Nightly) enthalten keinen
HIP-Adapter. Für die MI300A wird deshalb AdaptiveCpp 25.10 im Home-Verzeichnis
gebaut. Das geschieht im Job auf dem MI300A-Knoten, weil ROCm nur dort sicher
vorhanden ist.

```bash
# lokal (der Cluster hat kein Internet)
curl -fL -o AdaptiveCpp-25.10.0.tar.gz \
  https://github.com/AdaptiveCpp/AdaptiveCpp/archive/refs/tags/v25.10.0.tar.gz
scp AdaptiveCpp-25.10.0.tar.gz hk_seer1012@dachs-login.hs-esslingen.de:~
dachs push

# auf DACHS im Ordner ~/code
bash gemeinsam/dachs/amd.sh check     # optional: ROCm, Module, Werkzeuge
bash gemeinsam/dachs/amd.sh install   # einmalig, schreibt ~/sycl/env_amd.sh
bash gemeinsam/dachs/amd.sh messen    # Matmul, Stencil, Reduktion
```

Protokolle liegen in `ergebnisse_dachs/`, Ergebnisse in
`<Beispiel>/ergebnisse/dachs/mi300a_<jobid>/` (gleiches CSV-Format wie bei
L40S/H100). Lokal holen: `dachs pull Matmul/ergebnisse Matmul/` usw.
