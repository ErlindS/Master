# Matrixmultiplikation: von C++ zu SYCL in vier Stufen

Beispielprojekt für die Seminararbeit. Ein bestehendes C++-Programm
(C = A · B) wird schrittweise auf SYCL umgebaut. Jede Stufe führt genau
ein neues Konzept ein, alle Stufen messen gleich und prüfen ihr Ergebnis.

| Stufe | Datei | Was ändert sich? | Neue SYCL-Konzepte |
|---|---|---|---|
| 0 | `stufe0_sequentiell.cpp` | Ausgangsprogramm, 3 Schleifen, 1 CPU-Kern | – |
| 1 | `stufe1_usm_shared.cpp` | äußere Schleifen → `parallel_for`, gemeinsamer Speicher | Queue, Kernel, `range`/`id`, USM shared |
| 2 | `stufe2_usm_device.cpp` | Speicher getrennt, Kopieren explizit | `malloc_device`, `memcpy` |
| 3 | `stufe3_tiling.cpp` | Kacheln im schnellen Local Memory | `nd_range`, Work-Group, `local_accessor`, Barriere |
| 4 | *(kein neuer Code)* | Stufe 1–3 auf CPU, NVIDIA-, AMD-, Intel-GPU | Geräteauswahl, Backends |

Stufe 4 ist absichtlich nur eine andere Umgebungsvariable: genau das ist
das Versprechen von SYCL.

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

Kachelgröße für Stufe 3 ändern: `make clean && make TILE=32`

## Ausführen

```bash
./stufe3_tiling 1024        # eine Stufe, n = 1024
./run_benchmark.sh          # alle Stufen, n = 256 … 2048 -> ergebnisse.csv
```

Gerät wählen (Stufe 4):

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

## Die CSV-Spalten

| Spalte | Bedeutung |
|---|---|
| `erster_lauf_ms` | allererster Durchlauf, inkl. JIT-Kompilierung des Kernels und erstem Datentransfer |
| `median_ms` | Median der Messläufe, nur die Berechnung (bei Stufe 2/3 nur der Kernel) |
| `transfer_ms` | Stufe 2/3: Kopieren hin + zurück (Median) |
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
