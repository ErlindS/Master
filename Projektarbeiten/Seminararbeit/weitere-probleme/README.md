# Zwei weitere Fallbeispiele: 2D-Stencil und Reduktion

Ergänzung zur Matrixmultiplikation in `../sycl-matmul`. Die Matrixmultiplikation
ist **rechengebunden**. Die beiden Beispiele hier bilden das Gegenstück:

| Fallbeispiel | Charakter | Was es testet |
|---|---|---|
| 2D-Stencil (Wärmeleitung, Jacobi, 5 Punkte, 100 Zeitschritte) | speichergebunden, ca. 1 Operation je gelesenem Wert | Indexräume (2D/1D), lokaler Speicher mit Halo, Overhead pro Kernelstart |
| Reduktion (Summe von n Floats) | bandbreiten- und synchronisationsgebunden | Atomics, Kooperation in der Work-Group, Gruppenkollektive, eingebaute Reduktion |

Kennzahl ist die **effektive Bandbreite** in GB/s. Das ist die mindestens
nötige Datenmenge geteilt durch die Zeit: beim Stencil jeder Punkt einmal
gelesen und einmal geschrieben, bei der Reduktion jeder Wert einmal gelesen.
Jedes Ergebnis wird gegen eine Host-Referenz geprüft.

## Varianten

| Programm | Neues SYCL-Konzept | CUDA-Gegenstück (`cuda/`) |
|---|---|---|
| `stencil1_2d` | in-order-Queue, 2D-`range` | `cuda_stencil_global` |
| `stencil1_2d_sync` | wie oben, aber `q.wait()` nach jedem Zeitschritt | – |
| `stencil2_1d` | linearisierter 1D-Indexraum | – |
| `stencil3_lokal` | Kachel mit Halo im lokalen Speicher | `cuda_stencil_shared` |
| `reduktion1_atomar` | `atomic_ref` (ein Atomic je Work-Item) | `cuda_reduktion_atomar` |
| `reduktion2_lokal` | Baumreduktion im lokalen Speicher, Barrieren | `cuda_reduktion_shared` |
| `reduktion3_gruppe` | `reduce_over_group` | – |
| `reduktion4_builtin` | `sycl::reduction` | `cuda_reduktion_cub` (CUB-Bibliothek) |

## Bauen und messen

```bash
source ~/sycl/env.sh
make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA=--cuda-path=$CUDA_HOME
make -C cuda
./messen.sh                        # -> ergebnisse/<datum>_<rechner>.csv
```

Auf DACHS: `make -C cuda ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90"`.

## Ergebnisse RTX 5060 Ti und Ryzen 5 5600X (15.11.2026)

DPC++ 7.2, CUDA 13.4, Median aus 5 Läufen, effektive Bandbreite in GB/s.
Die Rohdaten stehen in `ergebnisse/2026-11-15_rtx5060ti_ryzen5600x.csv`.

### Stencil (n = 8192, 256 MB je Gitter, 100 Zeitschritte)

| Variante | RTX SYCL | RTX CUDA | Ryzen SYCL |
|---|---:|---:|---:|
| 2D, globaler Speicher | 357 | 368 | 15,6 |
| 2D, Warten nach jedem Schritt | 357 | – | 16,1 |
| 1D linearisiert | 224 | – | 11,7 |
| lokaler Speicher, Kachel 16 | 300 | 297 | 8,7 |

### Reduktion (n = 2^26, 256 MB; in Klammern n = 2^24)

| Variante | RTX SYCL | RTX CUDA | Ryzen SYCL |
|---|---:|---:|---:|
| Atomic je Work-Item | **0,03** (0,01) – 8,3 s! | 202 (53) | 2,4 |
| Baum im lokalen Speicher | 404 (95) | 419 (401) | 17,9 |
| `reduce_over_group` | 406 (107) | – | 17,1 |
| eingebaut (`sycl::reduction` / CUB) | 356 (296) | 419 (404) | 16,0 |

## Beobachtungen

1. **Speichergebundene Kernel portiert SYCL praktisch verlustfrei.** Der
   einfache Stencil erreicht 97 % der CUDA-Bandbreite, die Reduktion mit lokalem
   Speicher bei großem n 96–97 %. Anders als bei der Kachelung der
   Matrixmultiplikation (67 %) gibt es hier keinen nennenswerten Abstand.

2. **Float-Atomics sind auf NVIDIA der größte Stolperstein.** DPC++ übersetzt
   `atomic_ref<float>::fetch_add` in eine Compare-and-Swap-Schleife
   (`atom.relaxed.gpu.global.cas.b32` im PTX) statt in die native
   Float-Addition, die CUDAs `atomicAdd` nutzt. Bei 2^20 Work-Items auf
   dieselbe Speicherzelle braucht SYCL 8,3 s, CUDA 1,3 ms, ein Faktor von
   etwa 6000. Belegt ist der Befund am PTX, das aus dem Geräte-IR des Compilers
   mit `llc` für sm_120 erzeugt wurde. Auch die Reduktionen mit nur einem
   Atomic je Work-Group (4096 Stück) zeigen einen festen Zusatzaufwand von etwa
   0,5 ms: Bei n = 2^24 erreichen sie nur ein Viertel der CUDA-Bandbreite, bei
   n = 2^26 fällt der Aufwand nicht mehr ins Gewicht. `sycl::reduction` umgeht
   das Problem und ist bei kleinem n die schnellste SYCL-Variante.

3. **Lokaler Speicher lohnt sich beim Stencil nicht**, weder in SYCL noch in
   CUDA (−16 % bzw. −19 %). Die Caches der GPU fangen die fünffachen
   Lesezugriffe bereits ab, Barriere und Halo-Laden kosten dagegen Zeit. Auf
   der CPU halbiert die Variante die Bandbreite. Ob eine Optimierung hilft,
   hängt vom Problem ab, nicht vom Programmiermodell.

4. **1D statt 2D ist hier langsamer, auch auf der CPU** (11,7 statt 15,6 GB/s).
   Abram et al. (ISC 2025) berichten für ihre Kernel das Gegenteil. Hier
   kosten vermutlich Division und Modulo zur Indexberechnung mehr, als der
   einfachere Indexraum einspart.

5. **Der Overhead pro Kernelstart ist bei großen Kerneln vernachlässigbar.**
   Warten nach jedem Zeitschritt kostet bei n = 8192 nichts. Bei n = 1024
   (Probelauf, Kernel ca. 10 µs) sank die Bandbreite dagegen von 814 auf
   479 GB/s.

6. **Die CPU erreicht 16–18 GB/s**, die GPU mehr als das 20-Fache. Die
   Reihenfolge der Varianten ist auf CPU und GPU nicht dieselbe: Beim Stencil
   ist der lokale Speicher auf beiden Geräten am langsamsten, bei der Reduktion
   liegt die eingebaute Variante auf der GPU vorn, auf der CPU hinten.
