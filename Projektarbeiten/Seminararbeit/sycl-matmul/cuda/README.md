# Native CUDA-Vergleich

Gegenstücke der SYCL-Stufen in nativem CUDA. Sie schließen die Lücke „kein
Vergleich mit nativem CUDA“ aus den Grenzen der Arbeit. Die Kernel sind
Zeile für Zeile wie die SYCL-Kernel aufgebaut. Messung, Prüfung und
CSV-Ausgabe kommen unverändert aus `../common.hpp`.

| Programm | SYCL-Gegenstück | Begriffe |
|---|---|---|
| `cuda1_managed.cu` | Stufe 1 (USM shared) | `cudaMallocManaged` = `malloc_shared` |
| `cuda2_device.cu` | Stufe 2 + 3 (USM device, nd_range) | `cudaMalloc`/`cudaMemcpy` = `malloc_device`/`memcpy`; Block = Work-Group |
| `cuda4_tiling.cu` | Stufe 4 (Kachelung) | `__shared__` = `local_accessor`, `__syncthreads()` = `group_barrier` |
| `cuda_cublas.cu` | – (Obergrenze) | cuBLAS SGEMM, FP32 ohne TF32 (`CUBLAS_PEDANTIC_MATH`) |

CUDA verlangt bei jedem Kernelstart eine Blockgröße. Eine Entsprechung zum
einfachen SYCL-Indexraum (`range`), bei dem die Laufzeit die Work-Group-Größe
wählt, gibt es nicht. Deshalb fallen Stufe 2 und 3 in CUDA zusammen, und
`cuda1_managed` nutzt ebenfalls Blöcke von 16 × 16.

## Bauen und messen

```bash
export PATH=/usr/local/cuda/bin:$PATH
make all kacheln                 # -arch=native: für die GPU dieses Rechners
./run_cuda.sh                    # -> cuda.csv, n = 1024 und 2048

# DACHS (L40S = sm_89, H100 = sm_90), auf dem Login-Knoten bauen:
make ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90" all kacheln
```

## Erste Messung: RTX 5060 Ti (14.11.2026)

Gleicher Rechner, direkt nacheinander gemessen, Median aus 5 Läufen, GFLOPS.
Die Rohdaten stehen in `cuda_rtx.csv`.

| Variante | SYCL (DPC++ 7.2) n=1024 | CUDA 13.4 n=1024 | SYCL n=2048 | CUDA n=2048 | SYCL / CUDA (2048) |
|---|---:|---:|---:|---:|---:|
| Stufe 1 (shared/managed) | 1044 | 1556 | 1086 | 1518 | 0,72* |
| Stufe 3 (Block 16 × 16, global) | 1578 | 1437 | 1576 | 1536 | 1,03 |
| Stufe 4 (Kachel T = 16) | 1394 | 1899 | 1385 | 2054 | **0,67** |
| Stufe 4 (Kachel T = 32) | 1324 | 1821 | 1319 | 1932 | 0,68 |
| Stufe 4 (Kachel T = 64) | startet nicht | startet nicht | – | – | – |
| cuBLAS SGEMM | – | 7814 | – | 14333 | – |

\* SYCL-Stufe 1 lässt die Laufzeit die Work-Group-Größe wählen, CUDA nutzt
16 × 16. Der faire Vergleich desselben Kernels ist Stufe 3.

**Beobachtungen**

- Ohne lokalen Speicher (Stufe 3) ist SYCL gleich schnell wie natives CUDA.
  Die Abstraktion kostet hier nichts.
- Mit Kachelung erreicht SYCL nur rund zwei Drittel der CUDA-Leistung. In
  CUDA ist die Kachelung schneller als der einfache Kernel, in SYCL langsamer.
  Damit liegt die Ursache für das Verhalten von Stufe 4 auf RTX und L40S
  wahrscheinlich bei der Codeerzeugung von DPC++ für lokalen Speicher und
  Barrieren, nicht im Algorithmus. Ein Blick in das erzeugte PTX
  (`group_barrier` gegenüber `bar.sync`, Adressrechnung) wäre der nächste
  Schritt.
- T = 64 startet in beiden Modellen nicht (4096 Threads je Block, erlaubt
  sind 1024). Die Grenze kommt von der Hardware, nicht von SYCL.
- cuBLAS ist rund siebenmal so schnell wie der beste handgeschriebene Kernel.
  Beide Modelle lassen also gleich viel Leistung liegen.
