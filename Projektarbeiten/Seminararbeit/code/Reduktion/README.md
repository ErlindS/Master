# Fallbeispiel Reduktion (Atomics)

Ergänzung zur Matrixmultiplikation in `../Matmul`: die Summe von n Floats.
Die Reduktion ist **bandbreiten- und synchronisationsgebunden**. Getestet
werden atomare Operationen, Kooperation in der Work-Group, Gruppenkollektive
und die eingebaute Reduktion. Das zweite speichergebundene Beispiel, der
Stencil, liegt in `../Stencil`.

Kennzahl ist die **effektive Bandbreite** in GB/s: Jeder Wert wird einmal
gelesen, geteilt durch die Zeit. Jedes Ergebnis wird gegen eine
Host-Referenz geprüft.

## Varianten

Die Varianten unterscheiden sich darin, wie viele Atomics sie auf die eine
Ergebniszelle brauchen.

| Ordner | Programm | Neues SYCL-Konzept | Atomics auf `sum` | CUDA-Gegenstück |
|---|---|---|---|---|
| `1_Atomar/` | `reduktion1_atomar` | `atomic_ref`, ein Atomic je Work-Item | ca. 1 Million | `cuda_reduktion_atomar` |
| `2_Lokal/` | `reduktion2_lokal` | Baumreduktion im lokalen Speicher, Barrieren | eines je Work-Group | `cuda_reduktion_shared` |
| `3_Gruppe/` | `reduktion3_gruppe` | `reduce_over_group` | eines je Work-Group | – |
| `4_Eingebaut/` | `reduktion4_builtin` | `sycl::reduction` | entscheidet die Implementierung | `cuda_reduktion_cub` (CUB) |

Der gemeinsame Messrahmen (`sycl_rahmen.hpp`, `cuda_rahmen.cuh`,
`probleme.hpp`, `common.hpp`) liegt in `../gemeinsam`.

## Bauen und messen

Im Ordner `Reduktion`:

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

Achtung: `reduktion1_atomar` braucht auf L40S und H100 über 30 s je Lauf;
ein Job dauert dadurch rund 10 Minuten.

## Ergebnisse (n = 2^26, 256 MB)

DPC++ 7.2, Median aus 5 Läufen, effektive Bandbreite in GB/s. RTX/Ryzen:
CUDA 13.4, 15.11.2026, `ergebnisse/2026-11-15_rtx5060ti_ryzen5600x.csv`.
L40S/H100: DACHS, CUDA 12.8, 17.11.2026, `ergebnisse/dachs/<partition>_<job>/`
(zwei H100-Läufe, Abweichung ≤ 4 %).

| Variante | RTX SYCL | RTX CUDA | L40S SYCL | L40S CUDA | H100 SYCL | H100 CUDA | Ryzen SYCL |
|---|---:|---:|---:|---:|---:|---:|---:|
| Atomic je Work-Item | **0,03** (8,3 s) | 202 | **0,01** (31 s) | 158 | **0,01** (39 s) | 142 | 2,4 |
| Baum im lokalen Speicher | 404 | 419 | 206 | 738 | 111 | 2166 | 17,9 |
| `reduce_over_group` | 406 | – | 209 | – | 106 | – | 17,1 |
| eingebaut (`sycl::reduction` / CUB) | 356 | 419 | 551 | 715 | 1020 | 2792 | 16,0 |

Werte für n = 2^24 (64 MB) sind in den CSV-Dateien enthalten, eignen sich
aber nicht zum Vergleich: Sie passen in den L2-Cache der L40S (96 MB), CUDA
misst dort bis zu 2873 GB/s bei rund 864 GB/s Speicherbandbreite.

## Beobachtungen

1. **Float-Atomics sind auf NVIDIA der größte Stolperstein.** DPC++ übersetzt
   `atomic_ref<float>::fetch_add` in eine Compare-and-Swap-Schleife
   (`atom.relaxed.gpu.global.cas.b32` im PTX) statt in die native
   Float-Addition, die CUDAs `atomicAdd` nutzt. Bei 2^20 Work-Items auf
   dieselbe Speicherzelle braucht SYCL 8,3 s (RTX), 31 s (L40S) und 39 s
   (H100), CUDA 1,3–1,9 ms: ein Faktor von 6300 bis 20 500. Je mehr
   Rechenkerne konkurrieren, desto teurer wird die Schleife. Belegt ist der
   Befund am PTX, das aus dem Geräte-IR des Compilers mit `llc` für sm_120
   erzeugt wurde.

2. **Mit einem Atomic je Work-Group hängt das Ergebnis am Gerät.** Auf der RTX
   erreichen Baum und `reduce_over_group` 96–97 % von CUDA, auf der L40S nur
   28 %, auf der H100 5 %. Vermutlich wirkt auch hier die
   Compare-and-Swap-Schleife; belegt ist das nicht.

3. **`sycl::reduction` ist die robusteste SYCL-Variante.** Sie erreicht 85 %
   (RTX), 77 % (L40S) und 37 % (H100) von CUB und ist auf L40S und H100 die
   schnellste SYCL-Variante.

4. **Die Reihenfolge der Varianten ist auf CPU und GPU nicht dieselbe.** Die
   CPU erreicht 16–18 GB/s; die eingebaute Variante liegt dort hinten, auf
   den GPUs vorn.
