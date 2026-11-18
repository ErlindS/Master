# Code zur Seminararbeit „SYCL – Portable heterogene Programmierung mit C++“

| Ordner | Inhalt |
|---|---|
| `Matmul/` | Matrixmultiplikation, Stufe 0–5 (`StufeN/{Normal,Sycl,Cuda}`), cuBLAS-Referenz |
| `Stencil/` | 2D-Stencil, Varianten `1_2D`, `2_1D`, `3_Lokal` je in `Sycl/` und `Cuda/` |
| `Reduktion/` | Reduktion/Atomics, Varianten `1_Atomar` … `4_Eingebaut` je in `Sycl/` und `Cuda/` |
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
