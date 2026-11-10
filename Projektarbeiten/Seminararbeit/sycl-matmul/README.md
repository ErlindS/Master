# Matrixmultiplikation: von C++ zu SYCL in fünf Stufen

Beispielprojekt für die Seminararbeit. Ein bestehendes C++-Programm
(C = A · B) wird schrittweise auf SYCL umgebaut. Jede Stufe führt genau
ein neues Konzept ein, alle Stufen messen gleich und prüfen ihr Ergebnis.

| Stufe | Datei | Was ändert sich? | Neue SYCL-Konzepte |
|---|---|---|---|
| 0 | `stufe0_sequentiell.cpp` | Ausgangsprogramm, 3 Schleifen, 1 CPU-Kern | – |
| 1 | `stufe1_usm_shared.cpp` | äußere Schleifen → `parallel_for`, gemeinsamer Speicher | Queue, Kernel, `range`/`id`, USM shared |
| 2 | `stufe2_usm_device.cpp` | Speicher getrennt, Kopieren explizit | `malloc_device`, `memcpy` |
| 2b | `stufe2b_buffer.cpp` | Variante von 2: Buffer statt Zeiger, Kopieren implizit | `buffer`, `accessor`, Zugriffsmodi |
| 3 | `stufe3_ndrange.cpp` | Work-Group-Größe selbst festlegen, noch ohne Local Memory | `nd_range`, `nd_item`, Work-Group |
| 4 | `stufe4_tiling.cpp` | Kacheln im schnellen Local Memory | Command Group, `local_accessor`, Barriere |
| 5 | `stufe5_anpassung.cpp` | Kachelgröße zur Laufzeit passend zum Gerät wählen | Geräteabfragen, Specialization Constants, `sycl::exception`, Async Handler |

Der Gerätewechsel ist keine eigene Stufe: Jede Stufe läuft unverändert auf
CPU und GPU, gewählt wird nur über eine Umgebungsvariable (siehe unten).
Stufe 5 übergibt die Kachelgröße als Specialization Constant an den Kernel
aus Stufe 4, sortiert per Geräteabfrage aus, was das Gerät nicht erlaubt,
misst T = 8, 16, 32, 64 kurz und nimmt die schnellste. Details zur Auswahl
stehen auf stderr. AdaptiveCpp kennt `specialization_id` nicht; dort nutzt
der Code die Erweiterung `sycl::specialized` (`#ifdef __ACPP__`).

## Alles auf einmal messen (für die Arbeit)

```bash
./setup_dpcpp.sh                       # einmalig: DPC++ + OpenCL-CPU-Laufzeit nach ~/sycl, ohne sudo
./messen.sh                            # DPC++ finden, bauen, alle Stufen auf allen Geräten
SIZES="512 1024" REPS=3 ./messen.sh    # schneller Probelauf
```

Das Skript sucht DPC++ (`~/sycl/env.sh`, `/opt/intel/oneapi/setvars.sh`,
`icpx`), erkennt die Geräte per `sycl-ls`, baut passend (PTX für NVIDIA,
SPIR-V für die CPU), misst Stufe 0–5 je Gerät sowie das Kachelexperiment
inkl. Stufe 5 und schreibt alles nach `ergebnisse/<datum>_<rechner>/`.
Auf DACHS: `bash dachs/build.sh && sbatch dachs/messung.sbatch`.

## Bauen

**Intel oneAPI (DPC++)**, nach `source /opt/intel/oneapi/setvars.sh`:

```bash
make                                        # CPU / Intel-GPU
make SYCL_TARGETS=nvptx64-nvidia-cuda       # NVIDIA (CUDA-Plugin nötig)
```

**AdaptiveCpp**:

```bash
make COMPILER=acpp
```

Kachelgröße für Stufe 4 ändern: `make clean && make TILE=32`,
Work-Group-Größe für Stufe 3: `make WG=32`

## Ausführen

```bash
./stufe4_tiling 1024        # eine Stufe, n = 1024
./run_benchmark.sh          # alle Stufen, n = 256 … 2048 -> ergebnisse.csv
```

Gerät wählen:

```bash
# DPC++
ONEAPI_DEVICE_SELECTOR=cuda:gpu        ./run_benchmark.sh nvidia.csv
ONEAPI_DEVICE_SELECTOR=opencl:cpu      ./run_benchmark.sh cpu.csv
ONEAPI_DEVICE_SELECTOR=level_zero:gpu  ./run_benchmark.sh intel_gpu.csv
# AdaptiveCpp
ACPP_VISIBILITY_MASK=cuda ./run_benchmark.sh nvidia.csv
ACPP_VISIBILITY_MASK=omp  ./run_benchmark.sh cpu.csv
```

Verfügbare Geräte: `sycl-ls` (DPC++) bzw. `acpp-info` (AdaptiveCpp).

Weitere Optionen über Umgebungsvariablen: `SIZES="512 1024"`, `REPS=10`,
`SEQ_MAX=2048` (bis zu welchem n Stufe 0 mitläuft).

## Kachelgrößen-Experiment: Portabilität ≠ Performance-Portabilität

Stufe 4 wird einmal pro Kachelgröße gebaut und auf jedem Gerät gemessen:

```bash
make kacheln stufe5_anpassung               # stufe4_tiling_T8 … _T64
./run_kacheln.sh                            # -> kacheln.csv
python3 portabilitaet.py kacheln.csv        # Tabelle + PP-Kennzahl
```

Geräte, Kachelgrößen und n über `DEVICES`, `TILES`, `SIZES` (siehe Kopf
von `run_kacheln.sh`). `portabilitaet.py` berechnet je Gerät die Effizienz
(GFLOPS / beste Kachelgröße auf diesem Gerät) und daraus die
Performance-Portabilität nach Pennycook et al. (2016): den harmonischen
Mittelwert über alle Geräte, 0 sobald ein Gerät gar nicht läuft.

## Die CSV-Spalten

| Spalte | Bedeutung |
|---|---|
| `erster_lauf_ms` | allererster Durchlauf, inkl. JIT-Kompilierung des Kernels und erstem Datentransfer |
| `median_ms` | Median der Messläufe, nur die Berechnung (ab Stufe 2 nur der Kernel, bei 2b Kernel + Transfer) |
| `transfer_ms` | ab Stufe 2: Kopieren hin + zurück (Median), bei 2b nicht getrennt messbar (0) |
| `gflops` | 2·n³ / `median_ms` |
| `korrekt` | 64 zufällige Einträge gegen eine double-Referenz geprüft |

## Worauf man bei der Auswertung achten sollte

* **Ersten Lauf getrennt betrachten.** Er ist oft um ein Vielfaches
  langsamer (JIT-Kompilierung). Für die Folie „Schwierigkeiten“ gut
  geeignet, für den Performance-Vergleich aber der Median.
* **Transfer vs. Rechnung.** Bei kleinen n ist `transfer_ms` oft größer
  als `median_ms` – dann lohnt sich die GPU nicht. Wo die Kurven sich
  kreuzen, ist eine schöne Kernaussage.
* **Fairer Vergleich zu Stufe 0.** Stufe 0 nutzt nur einen Kern. Der
  Speedup gegenüber Stufe 1 auf der CPU zeigt also vor allem
  Mehrkern-Parallelität, nicht die GPU.
* **Kachelgröße.** `TILE=8/16/32` auf verschiedenen Geräten vergleichen:
  die beste Größe ist nicht überall dieselbe (Performance-Portabilität).
* **Stufe 1 auf der GPU.** Mit `malloc_shared` werden die Daten beim
  ersten Zugriff automatisch verschoben; der Median enthält das nicht
  mehr. Das erklärt, warum Stufe 1 im Median oft genauso schnell ist
  wie Stufe 2.

## Ein-Befehl-Setup (Docker, jedes Betriebssystem)

Wer nichts selbst installieren möchte, startet einfach:

```bash
./setup.sh                      # Linux, macOS, WSL2
```

```powershell
powershell -ExecutionPolicy Bypass -File .\setup.ps1     # Windows
```

Das Skript installiert bei Bedarf Docker (nach Rückfrage), erkennt eine
NVIDIA-GPU (unter Linux inkl. NVIDIA Container Toolkit), baut einmalig ein
Image mit LLVM 20 und AdaptiveCpp und führt danach Stufe 0–5 sowie das
Kachelgrößen-Experiment auf CPU und – falls vorhanden – GPU aus. Ergebnisse
landen in `ergebnisse/<datum>_<cpu|cuda>/` (`stufen_*.csv`, `kacheln.csv`,
`portabilitaet.txt`, `system.txt`).

| Option | Wirkung |
|---|---|
| `--yes` / `-Yes` | keine Rückfragen |
| `--cpu` / `-Cpu` | GPU ignorieren |
| `--rebuild` / `-Rebuild` | Image neu bauen |
| `SIZES`, `REPS`, `TILES`, `SEQ_MAX` | werden an die Benchmarks durchgereicht, z. B. `SIZES="512 1024" REPS=3 ./setup.sh` |

Hinweise: Im Container wird **AdaptiveCpp** statt DPC++ verwendet (frei
verfügbar, ohne Lizenz-Downloads). Die Zahlen sind daher nicht direkt mit
den DPC++-Messungen der Arbeit vergleichbar. Unter macOS ist nur die CPU
messbar. Für die GPU braucht der Host einen aktuellen NVIDIA-Treiber
(für CUDA 12.9: ≥ 575).
