# Fallbeispiel 2D-Stencil

Ergänzung zur Matrixmultiplikation in `../Matmul`. Die Matrixmultiplikation
ist **rechengebunden**, der Stencil ist das **speichergebundene** Gegenstück:
Wärmeleitung nach Jacobi mit 5 Punkten und 100 Zeitschritten, etwa eine
Operation je gelesenem Wert. Getestet werden Indexräume (2D/1D), lokaler
Speicher mit Halo und der Overhead pro Kernelstart. Das zweite speichergebundene
Beispiel, die Reduktion, liegt in `../Reduktion`.

Kennzahl ist die **effektive Bandbreite** in GB/s: Jeder Punkt wird einmal
gelesen und einmal geschrieben, geteilt durch die Zeit. Jedes Ergebnis wird
gegen eine Host-Referenz geprüft.

## Varianten

| Ordner | Programm | Neues SYCL-Konzept | CUDA-Gegenstück |
|---|---|---|---|
| `1_2D/` | `stencil1_2d` | in-order-Queue, 2D-`range` | `cuda_stencil_global` |
| `1_2D/` | `stencil1_2d_sync` | gleiche Quelle mit `-DSYNC_JEDE`: `q.wait()` nach jedem Zeitschritt | – |
| `2_1D/` | `stencil2_1d` | linearisierter 1D-Indexraum | – |
| `3_Lokal/` | `stencil3_lokal` | Kachel mit Halo im lokalen Speicher | `cuda_stencil_shared` |

Der gemeinsame Messrahmen (`sycl_rahmen.hpp`, `cuda_rahmen.cuh`,
`probleme.hpp`, `common.hpp`) liegt in `../gemeinsam`.

## Bauen und messen

Im Ordner `Stencil`:

```bash
source ~/sycl/env.sh
make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA=--cuda-path=$CUDA_HOME
make cuda
bash gemeinsam/messen.sh             # -> ergebnisse/<datum>_<rechner>.csv
```

Auf DACHS (Login-Knoten ohne GPU, daher feste Architekturen):

```bash
make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA="--cuda-path=$CUDA_HOME"
make cuda ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90"
mkdir -p ergebnisse/dachs
sbatch -p gpu1 gemeinsam/dachs.sbatch   # L40S
sbatch -p gpu8 gemeinsam/dachs.sbatch   # H100
```

## Ergebnisse (n = 8192, 256 MB je Gitter, 100 Zeitschritte)

DPC++ 7.2, Median aus 5 Läufen, effektive Bandbreite in GB/s. RTX/Ryzen:
CUDA 13.4, 15.11.2026, `ergebnisse/2026-11-15_rtx5060ti_ryzen5600x.csv`.
L40S/H100: DACHS, CUDA 12.8, 17.11.2026, `ergebnisse/dachs/<partition>_<job>/`
(zwei H100-Läufe, Abweichung ≤ 1 %).

| Variante | RTX SYCL | RTX CUDA | L40S SYCL | L40S CUDA | H100 SYCL | H100 CUDA | Ryzen SYCL |
|---|---:|---:|---:|---:|---:|---:|---:|
| 2D, globaler Speicher | 357 | 368 | 669 | 671 | 1552 | 1908 | 15,6 |
| 2D, Warten nach jedem Schritt | 357 | – | 661 | – | 1517 | – | 16,1 |
| 1D linearisiert | 224 | – | 483 | – | 425 | – | 11,7 |
| lokaler Speicher, Kachel 16 | 300 | 297 | 682 | 695 | 1237 | 765 | 8,7 |

## Beobachtungen

1. **Der Stencil portiert SYCL nahezu verlustfrei.** Die einfache Variante
   erreicht 97 % (RTX), 100 % (L40S) und 81 % (H100) der CUDA-Bandbreite.

2. **Ob lokaler Speicher sich lohnt, hängt vom Gerät ab.** Auf der RTX kostet
   er in SYCL und CUDA 16 bzw. 19 %, auf der L40S bringt er 2–4 %, auf der H100
   kostet er in SYCL 20 % und in CUDA sogar 60 %. Dort ist SYCL mit lokalem
   Speicher 62 % schneller als CUDA. Die Caches fangen die fünffachen
   Lesezugriffe meist schon ab, Barriere und Halo-Laden kosten Zeit. Auf der
   CPU halbiert die Variante die Bandbreite.

3. **1D statt 2D ist überall langsamer**: −37 % (RTX), −28 % (L40S), −73 %
   (H100), auch auf der CPU. Abram et al. (ISC 2025) berichten für ihre Kernel
   das Gegenteil. Hier kosten vermutlich Division und Modulo zur
   Indexberechnung mehr, als der einfachere Indexraum einspart.

4. **Der Overhead pro Kernelstart ist bei großen Kerneln vernachlässigbar.**
   Warten nach jedem Zeitschritt kostet bei n = 8192 höchstens 2 %. Bei
   n = 1024 (Probelauf auf der RTX, Kernel ca. 10 µs) sank die Bandbreite
   dagegen von 814 auf 479 GB/s.
