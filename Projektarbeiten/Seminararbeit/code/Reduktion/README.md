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
| `5_Gemeinsam/` | `reduktion5_gemeinsam` | gemeinsamer Speicher (USM shared), Kernel wie Variante 1 | ca. 1 Million | – |
| `6_Buffer/` | `reduktion6_buffer` | Buffer und Accessoren (Buffer je Messung neu) | ca. 1 Million | – |
| `6_Buffer/` | `reduktion6_buffer_wieder` | gleiche Quelle mit `-DBUFFER_WIEDER`: Buffer einmal angelegt | ca. 1 Million | – |
| `7_Geraet/` | `reduktion7_geraet` | Gruppengröße und Zahl der Work-Items aus Geräteabfragen, Fehlerbehandlung | eines je Work-Group | – |

### Zuordnung zu den Stufen der Matrixmultiplikation

| Stufe (Matmul) | Reduktion | isoliert damit |
|---|---|---|
| 1 – gemeinsamer Speicher | `reduktion5_gemeinsam` | Kosten des Speichermodells bei genau einem Lesezugriff je Wert |
| 2 – Gerätespeicher, explizit kopieren | `reduktion1_atomar` | Bezugspunkt aller übrigen Varianten |
| 2b – Buffer und Accessoren | `reduktion6_buffer`, `…_wieder` | Transferkosten (neu je Messung) gegenüber reinem Accessor-Aufwand (wiederverwendet) |
| 3/4 – Gruppenarbeit, lokaler Speicher | `reduktion2_lokal`, `reduktion3_gruppe` | Nutzen von Baumreduktion und Gruppenkollektiv |
| 5 – Parameter zur Laufzeit wählen | `reduktion7_geraet` | ob die Geräteabfrage die beste Gruppengröße findet |

Anders als beim Stencil braucht Variante 7 **keine** Specialization Constant:
Die Gruppengröße steht nur in der `nd_range`, im Kernel hängt keine Feldgröße
von ihr ab. `reduktion4_builtin` bleibt das Gegenstück zur Abstraktion ohne
eigene Kontrolle (vergleichbar mit Buffern bei der Matmul).

Parameter übergeben: `./reduktion7_geraet <n> <wiederholungen> [W]` (ohne `W`
wählt das Programm nach Geräteabfrage und kurzer Messung selbst). Die festen
Gruppengrößen der Varianten 2 und 3 lassen sich beim Übersetzen mit `-DWG`
ändern.

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

### Stufen-Gegenstücke (n = 2^26, DACHS, 19.11.2026)

`ergebnisse/dachs/gpu1_162867/` (L40S) und `gpu8_162868/` (H100), Median aus
5 Läufen, effektive Bandbreite in GB/s.

| Variante | Stufe | L40S | H100 | Atomics auf `sum` |
|---|---|---:|---:|---|
| `reduktion1_atomar` | 2 | 0,01 (31,0 s) | 0,01 (38,6 s) | 2^20 |
| `reduktion5_gemeinsam` | 1 | 0,01 (31,1 s) | 0,01 (38,3 s) | 2^20 |
| `reduktion6_buffer` | 2b, neu je Lauf | 0,01 (31,0 s) | 0,01 (38,5 s) | 2^20 |
| `reduktion6_buffer_wieder` | 2b, wiederverwendet | 0,01 (31,0 s) | 0,01 (38,5 s) | 2^20 |
| `reduktion2_lokal_W256` | 3/4 | 211 | 111 | 4096 |
| `reduktion3_gruppe_W256` | 3/4 | 213 | 106 | 4096 |
| `reduktion4_builtin` | Abstraktion | 607 | **827** | Implementierung |
| `reduktion7_geraet_W64` | 5 | 437 | 293 | ca. 1140 |
| `reduktion7_geraet_W256` | 5 | 529 | 403 | ca. 1140 |
| `reduktion7_geraet_W1024` | 5, selbst gewählt | **711** | 557 | ca. 1100 |
| `cuda_reduktion_shared_W256` | CUDA | 742 | 2103 | 4096 |
| `cuda_reduktion_cub` | CUDA/CUB | 721 | 2704 | – |

Zwei Befunde:

1. **Wo eine Pathologie dominiert, ist das Speichermodell gleichgültig.** Die
   vier Varianten mit einem Atomic je Work-Item liegen auf jedem Gerät innerhalb
   von 0,6 % – gemeinsamer Speicher, Gerätespeicher und Buffer machen keinen
   messbaren Unterschied, weil die Compare-and-Swap-Schleife auf 2^20
   konkurrierenden Zugriffen alles andere verdeckt. Dass die Laufzeit mit
   31 bzw. 38,5 s auch bei 16-fach kleinerem n gleich bleibt, bestätigt das:
   Sie hängt an der Zahl der Atomics, nicht am Datenvolumen.

2. **Die Zahl der atomaren Gruppenzugriffe ist der entscheidende Parameter.**
   Variante 7 leitet Gruppengröße und Work-Item-Zahl aus Geräteabfragen ab und
   kommt damit auf etwa 1140 statt 4096 Gruppen. Bei gleicher Gruppengröße
   (`W = 256`) bringt allein das 529 statt 213 GB/s auf der L40S. Zusätzlich
   hilft eine größere Gruppe: bei gleicher Gruppenzahl steigt die Bandbreite von
   437 (`W = 64`) auf 711 GB/s (`W = 1024`). In Summe erreicht die selbst
   gewählte Konfiguration auf der L40S **99 % von CUB** und schlägt die
   eingebaute Reduktion deutlich – auf der H100 dagegen nur 21 % von CUB und
   bleibt hinter `sycl::reduction` (31 %). Die beste SYCL-Strategie ist damit
   gerätespezifisch.

Achtung beim Nachmessen: Die vier Varianten mit einem Atomic je Work-Item
(1, 5, 6, 6\_wieder) brauchen auf L40S und H100 je über 30 s pro Lauf, ein
Messjob dauert dadurch rund 40 Minuten.

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
