# Fallbeispiel 2D-Stencil

Ergänzung zur Matrixmultiplikation in `../Matmul`. Die Matrixmultiplikation
ist **rechengebunden**, der Stencil ist das **speichergebundene** Gegenstück:
Wärmeleitung nach Jacobi mit 5 Punkten und 100 Zeitschritten, etwa eine
Operation je gelesenem Wert. Getestet werden die Speichermodelle (Gerätespeicher,
gemeinsamer Speicher, Buffer), Indexräume (2D/1D), Gruppengröße, lokaler
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
| `4_Gemeinsam/` | `stencil4_gemeinsam` | gemeinsamer Speicher (USM shared) statt Gerätespeicher | – |
| `5_Buffer/` | `stencil5_buffer` | Buffer und Accessoren, Abhängigkeiten automatisch (Buffer je Messung neu) | – |
| `5_Buffer/` | `stencil5_buffer_wieder` | gleiche Quelle mit `-DBUFFER_WIEDER`: Buffer einmal angelegt | – |
| `6_Gruppe/` | `stencil6_gruppe` | `nd_range` mit selbst gewählter Gruppengröße, **ohne** lokalen Speicher | – |
| `7_Geraet/` | `stencil7_geraet` | Kachelgröße zur Laufzeit: Geräteabfragen, Specialization Constant, Fehlerbehandlung; Halo-Laden ohne Division | – |
| `7_Geraet/` | `stencil7_geraet_flach` | gleiche Quelle mit `-DLADEN_FLACH`: Halo-Laden mit Division wie Variante 3 | – |

### Zuordnung zu den Stufen der Matrixmultiplikation

Die Varianten bilden dieselbe Stufenfolge wie `../Matmul`, damit sich
vergleichen lässt, welcher Schritt bei einem **speichergebundenen** Problem
etwas bringt und welcher nicht:

| Stufe (Matmul) | Stencil | isoliert damit |
|---|---|---|
| 1 – gemeinsamer Speicher | `stencil4_gemeinsam` | Kosten des bequemen Speichermodells, hier nicht von Rechenzeit verdeckt |
| 2 – Gerätespeicher, explizit kopieren | `stencil1_2d` | Bezugspunkt aller übrigen Varianten |
| 2b – Buffer und Accessoren | `stencil5_buffer`, `…_wieder` | Transferkosten (neu je Messung) gegenüber reinem Accessor-Aufwand (wiederverwendet) |
| 3 – Gruppengröße selbst festlegen | `stencil6_gruppe` (Parameter `W`) | Wirkung der Gruppeneinteilung allein |
| 4 – lokaler Speicher, feste Kachel | `stencil3_lokal` (`-DBLOCK`) | Nutzen des lokalen Speichers gegenüber Stufe 3 |
| 5 – Parameter zur Laufzeit wählen | `stencil7_geraet`, `…_flach` | ob die Geräteabfrage die beste Kachel findet und was die Laufzeitgröße kostet |

Die Varianten `stencil1_2d_sync` (Startlatenz) und `stencil2_1d`
(Indexraum) haben kein Gegenstück bei der Matrixmultiplikation; sie prüfen
Fragen, die erst beim Stencil auftreten.

Warum Variante 7 zweimal gebaut wird: DPC++ emuliert Specialization Constants
auf dem CUDA-Backend über einen Parameterpuffer, die Kachelgröße ist dem
PTX-Compiler also keine Konstante. Die Vermutung war, dass vor allem die
Ganzzahl-Division im Halo-Laden (`k / (B + 2)`) teuer wird, die mit festem `B`
wegoptimiert wird. Die Messung widerlegt das: Beide Ladevarianten
unterscheiden sich um höchstens 0,9 % (siehe unten). Die Kosten entstehen
anderswo – sie sind pro Kernelstart konstant.

Parameter übergeben: `./stencil6_gruppe <n> <wiederholungen> <W>` und
`./stencil7_geraet <n> <wiederholungen> [B]` (ohne `B` wählt das Programm
selbst). `messen.sh` fährt `GRUPPEN="8 16 32"` für die Gruppengröße ab.

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

### Stufen-Gegenstücke (n = 8192, DACHS, 19.11.2026)

`ergebnisse/dachs/gpu1_162865/` (L40S) und `gpu8_162866/` (H100), Median aus
5 Läufen. Bezug ist `stencil1_2d` (Stufe 2, eigener Gerätespeicher).

| Variante | Stufe | L40S | zu Stufe 2 | H100 | zu Stufe 2 |
|---|---|---:|---:|---:|---:|
| `stencil4_gemeinsam` | 1 | 306 | −54 % | 410 | −74 % |
| `stencil5_buffer` | 2b, neu je Lauf | 421 | −37 % | 623 | −60 % |
| `stencil5_buffer_wieder` | 2b, wiederverwendet | 515 | −23 % | 878 | −43 % |
| `stencil1_2d` | 2 | 668 | – | 1553 | – |
| `stencil6_gruppe_W8` | 3 | 670 | ±0 % | 848 | −45 % |
| `stencil6_gruppe_W16` | 3 | 669 | ±0 % | **1931** | **+24 %** |
| `stencil6_gruppe_W32` | 3 | **700** | **+5 %** | 1569 | +1 % |
| `stencil3_lokal_B16` | 4 | 682 | +2 % | 1238 | −20 % |
| `stencil7_geraet_flach_B16` | 5, mit Division | 592 | −12 % | 716 | −54 % |
| `stencil7_geraet` | 5, ohne Division | 587 | −12 % | 716 | −54 % |
| `cuda_stencil_global_B16` | CUDA | 671 | | 1910 | |
| `cuda_stencil_shared_B16` | CUDA | 695 | | 762 | |

Drei Befunde:

1. **Die Work-Group-Größe ist der stärkste Hebel, und die Vorgabe der Laufzeit
   ist auf der H100 schlecht.** Mit `W = 16` erreicht SYCL dort 1931 GB/s, also
   101 % des nativen CUDA-Kernels, während der einfache `range`-Indexraum
   (Variante 1) bei 81 % bleibt. Die bisher berichtete Lücke zu CUDA auf der
   H100 ist also keine Frage der Codeerzeugung, sondern der voreingestellten
   Gruppengröße. Zugleich ist der Parameter heikel: `W = 8` kostet auf der H100
   45 %, auf der L40S nichts.

2. **Die Kosten der emulierten Specialization Constant sind pro Kernelstart
   konstant, nicht proportional zur Arbeit.** Die Differenz zu Variante 3 mit
   festem `B` beträgt auf der L40S 120,2 µs je Zeitschritt bei n = 4096 und
   120,1 µs bei n = 8192 – bei vierfacher Datenmenge derselbe Absolutwert; auf
   der H100 262 bzw. 317 µs. Relativ sieht das sehr unterschiedlich aus
   (−38 % bei n = 4096, −13 % bei n = 8192 auf der L40S), ist aber derselbe
   Effekt. Die Ganzzahl-Division spielt keine Rolle (≤ 0,9 % Unterschied
   zwischen beiden Ladevarianten).

3. **Beide bequemen Speichermodelle kosten bei einem speichergebundenen Problem
   viel**, und zwar umso mehr, je schneller das Gerät ist: gemeinsamer Speicher
   −54 % (L40S) und −74 % (H100), Buffer −37 % bzw. −60 %. Werden die Buffer
   wiederverwendet, bleibt noch −23 % bzw. −43 % – der Rest war Datentransfer.
   Bei der Matrixmultiplikation gehen dieselben Mechanismen in der Rechenzeit
   unter.

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
