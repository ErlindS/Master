# Versuchsaufbau: Performance-Portabilität von SYCL

Erweiterter Versuchsaufbau zur Seminararbeit. Er beantwortet die Kritikpunkte
aus Abschnitt 5.3 (fehlende Baseline, nur ein Compiler, nur ein Kernel,
schwache Statistik) und prüft vorab festgelegte Hypothesen.

## Hypothesen

Die Schwellen stehen oben in `auswertung.py` und werden **vor** der Messung
festgelegt.

| | Hypothese | Gemessen durch |
|---|---|---|
| H1 | Die minimale Portierung (`naiv`) erreicht auf der GPU ≥ 70 % der besten eigenen SYCL-Variante. | `sycl_gemm naiv` vs. `tileT` |
| H2 | Bei identischem Algorithmus ist SYCL höchstens 10 % langsamer als CUDA. | `sycl_gemm tileT` vs. `cuda_gemm tileT` |
| H3 | Keine feste Kachelgröße ist auf allen Geräten optimal; eine gerätebewusste Auswahl (`auto`) ist performance-portabler. | PP für `tile8…64` vs. `auto` |
| H4 | Die Wahl der SYCL-Implementierung (DPC++ vs. AdaptiveCpp) verändert die Leistung um mehr als 10 %. | gleiche Variante, beide Compiler |
| H5 | Der speichergebundene Kernel ist portabler: SYCL erreicht ≥ 90 % der nativen Bandbreite. | `sycl_triad` vs. `cuda_triad` / `cpu_triad` |

Zusätzlich: Abstand zur Bibliothek (cuBLAS / OpenBLAS), Anlaufkosten (erster
Lauf), Umstellungsaufwand in Codezeilen.

## Programme

| Programm | Inhalt | Rolle |
|---|---|---|
| `sycl_gemm.cpp` | `naiv`, `tile8/16/32/64`, `auto` (gerätebewusstes Autotuning über `get_info`) | Untersuchungsgegenstand |
| `sycl_triad.cpp` | STREAM-Triade `a = b + s·c` | speichergebundener Vergleichskernel |
| `cuda_gemm.cu` | dieselben Kernel in CUDA + `cublasSgemm` | Kosten der Portabilität, GPU-Baseline |
| `cuda_triad.cu` | Triade in CUDA | native GPU-Referenz |
| `cpu_blas.cpp` | OpenBLAS `cblas_sgemm` | CPU-Baseline |
| `cpu_triad.cpp` | Triade mit OpenMP | native CPU-Referenz |

Alle Programme schreiben **Rohdaten** (eine Zeile pro Messung) im selben
CSV-Format, siehe Kopf von `common.hpp`. Die SYCL- und CUDA-Programme messen
die Kernel-Zeit über Events (ohne Overhead auf dem Host); der erste Lauf
inkl. JIT wird getrennt mit der Host-Uhr erfasst.

## Bauen

Auf dem Messrechner (Ryzen 5 5600X + RTX 5060 Ti):

```bash
sudo apt install libopenblas-dev        # einmalig, für cpu_blas
make dpcpp DPCPP_CXX=/pfad/zu/dpcpp/bin/clang++   # eigener DPC++-Build mit CUDA
make cuda                                # braucht nvcc + cuBLAS
make cpu
make acpp                                # optional, falls AdaptiveCpp installiert
```

DPC++ übersetzt standardmäßig für `nvptx64-nvidia-cuda,spir64`, also für GPU
**und** CPU in einer Datei. Für AOT auf Blackwell:
`DPCPP_EXTRA="-Xsycl-target-backend=nvptx64-nvidia-cuda --cuda-gpu-arch=sm_120"`.

## Ausführen

```bash
# Probelauf (wenige Minuten)
RUNS=2 REPS=5 SIZES="512 1024" TRIAD_N="16777216" ./run_all.sh

# vollständiger Versuchsplan
./run_all.sh                        # -> messdaten/roh_<datum>.csv (+ _system.txt, .log)
```

Versuchsplan: jede Konfiguration in `RUNS=5` unabhängigen Programmstarts mit
je `REPS=20` Wiederholungen; die Reihenfolge der Konfigurationen wird pro Lauf
reproduzierbar gemischt (`SEED`), damit Erwärmung oder Taktänderungen nicht
systematisch eine Variante treffen. Fehlende Programme werden übersprungen.

Vor dem Messen empfohlen:

```bash
sudo cpupower frequency-set -g performance   # CPU-Governor festsetzen
nvidia-smi -q -d CLOCK                       # GPU-Takt notieren
```

Andere Programme schließen, Laptop am Netzteil, nicht während der Messung am
Rechner arbeiten.

## Auswerten

```bash
python3 auswertung.py messdaten/roh_<datum>.csv   # -> ergebnisse/roh_<datum>/
python3 aufwand.py                                # -> ergebnisse/aufwand.csv
```

| Ausgabe | Inhalt |
|---|---|
| `konfigurationen.csv` | Median, 95-%-Konfidenzintervall (zweistufiger Bootstrap über Läufe und Wiederholungen), Leistung, erster Lauf, gewähltes T bei `auto` |
| `effizienz.csv` | Effizienz relativ zur Bibliothek und zur besten eigenen Variante |
| `performance_portabilitaet.csv` | PP nach Pennycook je Variante, für beide Effizienzbegriffe, inkl. `auto` und „beste Kachel je Gerät“ |
| `vergleiche.csv` | SYCL/CUDA, SYCL/native Triade, DPC++/AdaptiveCpp mit Angabe, ob sich die Intervalle überlappen |
| `zusammenfassung.md` | H1–H5 mit Ergebnis (bestätigt / widerlegt) |
| `plots/*.dat` | Datendateien für `pgfplots` (`\addplot table {…}`) |

## Zuordnung zur Seminararbeit

| Kapitel | Inhalt aus diesem Aufbau |
|---|---|
| 3 Vorgehen | Hypothesen-Tabelle, Versuchsplan (Läufe, Wiederholungen, Mischung), Baselines |
| 4 Umsetzung | `auto`-Variante (gerätebewusste Auswahl), Triade, CUDA-Pendant, `aufwand.csv` |
| 5 Ergebnisse | `zusammenfassung.md`; Abbildung PP fest vs. `auto` aus `plots/pp_*.dat`; Effizienz gegenüber Bibliothek aus `plots/effizienz_*.dat` |
| 5.3 Grenzen | verbleibende Einschränkungen (zwei Geräte, eine Problemklasse je Kernel) |
