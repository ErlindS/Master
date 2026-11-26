# Sprechtext: SYCL – Ein C++-Quellcode für jede Hardware?

> **Gesamtdauer:** 20 Minuten (4 Min. Einstieg & Forschungsfragen · 2 Min. Methodik · 8 Min. Konzepte Stufe 1–5 · 6 Min. Ergebnisse, PTX-Analyse & Fazit)
> **Tipp für den Vortrag:** Öffne [sycl_praesentation.html](file:///home/debian/repo/Master/Seminararbeit/Praesentation/sycl_praesentation.html) und drücke **`P`** für das separate **Referentenfenster** auf deinem Laptop-Bildschirm (mit großer Uhr, Soll-Zeitfenster und exakt zum Klick passendem Sprechtext), während auf dem Beamer nur die ruhigen Folien ohne ablenkende Leisten zu sehen sind.

---

## 1. Einstieg & Forschungsfragen (4 Minuten | 00:00–04:00)

### Einstieg & Das Problem der Herstellerbindung (ca. 2:15 Min. | Folien 1–3)

**[Folie 1: Titelfolie]**

Hallo zusammen und herzlich willkommen zu meinem Vortrag über **SYCL** – und die Frage, ob ein einziger C++-Quellcode in der Praxis wirklich auf jeder Hardware schnell laufen kann. Steigen wir direkt mit einem kurzen Gedankenexperiment ein.

**[Folie 2: Storyboard – Vendor Lock-in]**

Stellt euch vor: Ihr seid ein talentierter Softwareentwickler und wollt eure rechenintensive Simulation massiv beschleunigen.
- **[Klick 1]** Weil ihr bestimmten Menschen in Lederjacken vertraut habt …
- **[Klick 2]** … habt ihr euch entschieden, gleich mehrere NVIDIA-Grafikkarten zu kaufen und euren gesamten Code in CUDA zu schreiben.
- **[Klick 3]** Doch wie es so kam, sind genau diese Karten immer teurer geworden – die Preise explodieren, oder ihr habt schlichtweg das Vertrauen in den Hersteller verloren.
- **[Klick 4]** Also kauft ihr euch stattdessen eine Grafikkarte von AMD.
- **[Klick 5]** Ihr baut sie ein, wollt kompilieren – und lauft direkt gegen eine Wand: Euer NVIDIA-Code startet auf AMD nicht! Für AMD bräuchte man statt CUDA nämlich **HIP**, für Intel **Level Zero** und für CPUs wieder etwas anderes.

**[Folie 3: Was ist SYCL? Das Bild wird repariert]**

Hier seht ihr dieses Dilemma als Architektur: Euer Quellcode steckt in `programm.cu` fest und erreicht nur NVIDIA – zu allen anderen Plattformen sind die Verbindungen gekappt.

Das ist erst mal ziemlich mies – aber Hilfe ist vor Ort und heißt **SYCL** **[Klick 1 – Diagramm repariert sich live]**.

SYCL ist kein einzelner Compiler, sondern ein offener Standard der **Khronos Group** auf Basis von normalem Standard-C++17 für **heterogene** Systeme. „Heterogen“ bedeutet hier „unterschiedlich“ – das Gegenteil von homogen. Zwischen eurem C++-Code und der Hardware sitzt eine SYCL-Implementierung – die beiden wichtigsten sind **AdaptiveCpp** von der Universität Heidelberg und **DPC++** von Intel. Beide unterscheiden sich unter der Haube leicht, erfüllen aber denselben Standard und bringen denselben Code auf NVIDIA, AMD, Intel und normale Multicore-CPUs.

**[Klick 2]** Warum nimmt man in **Großrechenzentren** dafür nicht einfach OpenMP oder OpenCL? Weil neben Herstellerbindung und doppelten Codebasen noch ein drittes Problem existiert: der Konflikt zwischen **Abstraktion und Kontrolle**. OpenMP ist bequem, erlaubt aber kaum Zugriff auf die Speicherhierarchie; CUDA und OpenCL geben volle Kontrolle, kosten aber Portabilität oder Produktivität. SYCL verspricht beides zugleich.

---

### Forschungsfragen F1 & F2 (ca. 1:45 Min. | Folie 4)

**[Folie 4: Forschungsfragen F1 & F2]**

Jetzt ist „läuft überall fehlerfrei“ – also **funktionale Portabilität** – in der Praxis etwas völlig anderes als „läuft überall *schnell*“, also echte **Performance-Portabilität**. Darauf bauen meine beiden Forschungsfragen auf:

**[Klick 1]** **Forschungsfrage F1:** *„Welche Konzepte werden einem zur Portierung von Programmen zur Verfügung gestellt?“*
Wie gelingt der Umbau von sequentiellem C++ Schritt für Schritt – und wie löst SYCL dabei den Spagat zwischen bequemer Abstraktion und hardwarenaher Kontrolle?

**[Klick 2]** **Forschungsfrage F2:** *„Existieren unterschiedliche Implementierungsgrade von SYCL, bei denen man am Ende doch die Hardware berücksichtigen muss?“*
Wo reicht portabler Code aus – und wo zwingen Hardwaregrenzen, Problemklasse und Compiler-Backends doch wieder zur Rücksicht auf das konkrete Gerät?

---

## 2. Methodik (2 Minuten | 04:00–06:00)

### Drei gegensätzliche Flaschenhälse & fünf Stufen (1 Min. | Folie 5)

**[Folie 5: Drei Problemklassen × 5 Stufen]**

Um das belastbar zu messen, habe ich nicht nur ein einziges Beispiel gewählt, sondern **drei gegensätzliche Hardware-Flaschenhälse** implementiert:
1. **Matrixmultiplikation** – extrem viele Berechnungen pro Wert, also klassisch *rechengebunden*.
2. **2D-Stencil** – eine Wärmeleitungssimulation über 100 Zeitschritte im 5-Punkt-Gitter; wenig Rechnung, ständiger Datenhunger, also *speichergebunden*.
3. **Reduktion** – das Aufsummieren von über 67 Millionen Zahlen auf einen einzigen Wert; hier konkurrieren Millionen Threads am Ausgang, also *synchronisationsgebunden*.

**[Klick 1]** Alle drei Probleme durchlaufen dieselben **fünf aufbauenden Portierungsstufen** – mit jeder Stufe greife ich tiefer in den SYCL-Werkzeugkasten, von maximaler Bequemlichkeit links bis zu expliziter Hardware-Anpassung rechts.

---

### Testsysteme & DACHS-Cluster (1 Min. | Folie 6)

**[Folie 6: Vier Zielgeräte]**

Ausgeführt habe ich jede Stufe mit **DPC++** auf vier sehr unterschiedlichen Geräten – und zwar mit einem einzigen Binary pro Rechner, umgeschaltet erst beim Programmstart: lokal auf meinem **AMD Ryzen 5 5600X** und meiner **RTX 5060 Ti mit 16 GB**, sowie auf dem **DACHS-Cluster** der Hochschule Esslingen auf einer **NVIDIA L40S** und einer **H100 mit 80 GB HBM3-Speicher**.

Gemessen wurden jeweils der Median aus fünf Läufen – der allererste Lauf wegen der Just-in-Time-Übersetzung getrennt – in **Millisekunden**, **GFLOPS** beziehungsweise **GB/s**, verglichen gegen sequentielles C++, handgeschriebenes **CUDA** sowie NVIDIAs **cuBLAS** und **CUB**.

---

## 3. Konzepte: Stufe 1 bis 5 (8 Minuten | 06:00–14:00)

### Stufe 1: Aus zwei Schleifen wird ein GPU-Kernel (ca. 1:40 Min. | Folie 7)

**[Folie 7: Stufe 1 – Queue, Kernel & Work-Items]**

Gehen wir die fünf Stufen am roten Faden der **Matrixmultiplikation** durch. Rechts oben seht ihr unser Ausgangsprogramm (Stufe 0): drei verschachtelte `for`-Schleifen über `i`, `j` und `k` auf einem einzigen CPU-Kern.

Links seht ihr die Rollenverteilung in SYCL: Der **Host** – unsere CPU, auf der `main()` startet – legt eine **`sycl::queue`** an (eine Warteschlange, genau wie aus *Algorithmen und Datenstrukturen*) und delegiert darüber einen **Kernel** an das **Gerät**.

**[Klick 1 – Code-Transformation]** Und rechts unten seht ihr, wie minimal-invasiv Stufe 1 ist: Host- und Gerätecode stehen in **derselben C++-Datei** (*Single-Source*). Wir ersetzen lediglich die beiden äußeren Schleifen über `i` und `j` durch ein `q.parallel_for` über einen zweidimensionalen **Indexraum**. Jede einzelne Ausführung darin heißt **Work-Item** – bei einer $2048\times 2048$-Matrix rechnen damit auf Schlag **4,2 Millionen Work-Items** parallel, während die innere `k`-Schleife unverändert bleibt!

---

### Stufe 2 (& 2b): Wer steuert den Speicherbus? (ca. 1:40 Min. | Folie 8)

**[Folie 8: Stufe 2 – USM vs. Buffer]**

In **Stufe 2** schauen wir auf die getrennten Speicher von Host (RAM) und GPU (VRAM). SYCL bietet hier zwei Welten an: **Unified Shared Memory (USM)** über gewohnte Pointer und **Buffer** über Container.

Bei **USM** gibt es zwei zentrale Modi:
- **Shared Allocation (`malloc_shared`)** aus Stufe 1 gibt uns einen gemeinsamen Pointer für Host und GPU und verschiebt Speicherseiten bei Zugriff automatisch – super bequem, versteckt aber, wann kopiert wird.
- **Device Allocation (`malloc_device`)** in **Stufe 2** legt die Daten fest in den schnellen Grafikspeicher. Wir kopieren sie explizit per `q.memcpy` hin und zurück – ein paar Zeilen mehr Code, aber volle Kontrolle und saubere Trennung von Transfer- und Rechenzeit!

**[ggf. Taste T drücken]** Die Alternative **Stufe 2b** sind **Buffer**: Man verpackt die Daten in einen Container, das Gerät fordert sie über **Accessoren** (`read_only` / `write_only`) an, und die SYCL-Laufzeit regelt alle Kopien im Hintergrund selbst.

---

### Stufe 3: Abhängigkeitsgraph & eigene Work-Groups (ca. 1:35 Min. | Folie 9)

**[Folie 9: Stufe 3 – Abhängigkeiten & Work-Groups]**

**Stufe 3** bringt zwei strukturelle Werkzeuge:
Erstens **Abhängigkeiten** links: In echten Programmen laufen mehrere Schritte nacheinander und nebeneinander. Da SYCL-Queues standardmäßig asynchron (*out-of-order*) arbeiten, können `Kopie A` und `Kopie B` gleichzeitig über den Bus laufen; über Events und **`depends_on`** wartet der Kernel exakt auf beide, bevor danach `Kopie C` startet. Für rein iterative Ketten wie unsere 100 Stencil-Schritte kann man alternativ eine strikte *In-Order-Queue* wählen.

Zweitens wechseln wir rechts im Code von einem flachen Bereich auf **`sycl::nd_range`**: Statt der Laufzeit die Aufteilung zu überlassen, gruppieren wir den Indexraum selbst in **Work-Groups** der festen Größe $W \times W$ – die direkte Vorstufe für Stufe 4.

---

### Stufe 4: Kachelung (Tiling) im schnellen Local Memory (ca. 1:40 Min. | Folie 10)

**[Folie 10: Stufe 4 – Kachelung & Local Memory]**

In **Stufe 4** nutzen wir einen klassischen HPC-Trick, den man genauso aus CUDA kennt: **Tiling** in einen globalen und lokalen Raum.

Warum bringt das so viel? In Stufe 1 bis 3 musste jedes einzelne Work-Item seine komplette Zeile von `A` und Spalte von `B` immer wieder aus dem großen, aber **langsamen globalen Grafikspeicher** holen. In Stufe 4 arbeiten alle Work-Items einer Work-Group als Team auf derselben Recheneinheit zusammen und teilen sich deren winzigen, **extrem schnellen On-Chip Local Memory** (`local_accessor` in einer *Command Group* `q.submit`):
1. Jedes Work-Item lädt nur *einen* Wert in die gemeinsame lokale Kachel.
2. Alle warten an einer **Barriere** (`sycl::group_barrier`), bis die Kachel komplett gefüllt ist.
3. Alle rechnen ihre Teilprodukte blitzschnell lokal aus – und synchronisieren erneut, bevor die nächste Kachel geladen wird.

Bei $16\times 16$-Kacheln sparen wir so **Faktor 16** an globalem Speicherverkehr – und treffen auf der CPU perfekt den L1/L2-Cache!

---

### Stufe 5: Wenn derselbe Code auf echte Hardwarelimits prallt (ca. 1:25 Min. | Folie 11)

**[Folie 11: Stufe 5 – Kachelgröße zur Laufzeit]**

**Stufe 5** stellt nun die Schlüsselfrage für F2: *Welche Kachelgröße $T$ schreiben wir überhaupt in den Code?*
Wählen wir $T = 64$, sind das $64 \times 64 = 4096$ Work-Items pro Gruppe. Unsere CPU liebt das – aber jede getestete GPU hat ein hartes Hardwarelimit von **1024 Work-Items pro Gruppe** und bricht sofort mit einem Laufzeitfehler ab!

Stufe 5 liest diese Hardwaregrenzen deshalb vorab per **`get_info`** aus, sortiert $T = 64$ auf GPUs sauber aus, misst die erlaubten Kandidaten kurz an und übergibt den Sieger per **Specialization Constant** an den JIT-Compiler. Schon am Code sehen wir also: Ganz ohne Blick auf die Hardware geht es nicht.

---

## 4. Ergebnisse und Schluss (6 Minuten | 14:00–20:00)

### Die 4 zentralen Erkenntnisse in Zahlen (4 Min. | Folien 12–15)

**[Folie 12: Ergebnis 1 – Matrixmultiplikation]**

Kommen wir zu den Messwerten! Die gute Nachricht vorweg: **Alle Programme liefen ohne eine einzige Zeile Codeänderung fehlerfrei auf allen vier Geräten.** Aber schauen wir uns vier markante Zahlen an:

**Zahl 1: Bis zu 15.600-facher Speedup ab Stufe 1 – aber kein universeller Sieger.**
Von 0,4 GFLOPS auf einem CPU-Kern springt schon Stufe 1 auf bis zu **6.044 GFLOPS** auf der H100. Bei $n = 2048$ zeigt sich aber: Auf der **H100 (6.348 GFLOPS)** und dem **Ryzen (3,5× Sprung auf 176 GFLOPS!)** gewinnt das Tiling. Auf **RTX 5060 Ti** und **L40S** ist dagegen **Stufe 3 *ohne* Local Memory** rund 11 % schneller als Stufe 4! Buffer (2b) kosten überall bis zu 64 % mehr Zeit **[optional Taste T]** – und je schneller die GPU rechnet, desto teurer wird das Kopieren: Auf der H100 macht der PCIe-Transfer schon **74 % der Kernelzeit** aus!

**[Folie 13: Ergebnis 2 – Performance-Portabilität]**

**Zahl 2: Performance-Portabilität `0,00` bei `T = 64` gegenüber `0,97` bei `T = 16`.**
Auf allen GPUs gewinnt $T = 16$, auf der CPU bei $n = 2048$ aber ausgerechnet $T = 64$ – und weil $T = 64$ auf keiner GPU startet, fällt die Performance-Portabilität nach Pennycook sofort auf **null**. Stufe 5 verhindert diesen Crash zuverlässig (`PP = 0,94`), verliert auf den GPUs im Dauerbetrieb aber 5 bis 43 %, weil DPC++ *Specialization Constants* auf CUDA über einen versteckten Parameterpuffer emulieren muss (auf der CPU mit nativem SPIR-V: 0 % Verlust!).

**[Folie 14: Ergebnis 3 – 2D-Stencil]**

**Zahl 3: Beim Speicherproblem kostet Bequemlichkeit 74 % Bandbreite – und eine Zeile bringt 101 % von CUDA.**
Beim speichergebundenen **Stencil** kippen alle Gewichte: `malloc_shared`, bei der Matrixmultiplikation noch völlig kostenlos, frisst auf der H100 plötzlich **74 % Bandbreite**, Buffer fressen 43 bis 60 %. Und der stärkste Hebel ist Stufe 3: Überlässt man die Gruppeneinteilung der Laufzeit, erreicht man nur 81 % von CUDA – setzt man `W = 16` selbst, springt SYCL auf **1.931 GB/s, also 101 % von nativem CUDA**!

**[Folie 15: Ergebnis 4 – Reduktion, PTX & CUDA-Gesamtvergleich]**

**Zahl 4: Faktor 20.500 Unterschied durch eine einzige PTX-Schleife bei der Reduktion.**
Wenn bei der **Reduktion** jedes Work-Item per `atomic_ref<float>` auf das Ergebnis addiert, braucht CUDA auf der H100 **1,9 Millisekunden** – unser SYCL-Code **39 Sekunden**! Ich habe mir den erzeugten PTX-Code angeschaut und den Grund gefunden: Statt NVIDIAs nativer Hardware-Instruktion `atom.global.add.f32` erzeugt DPC++ eine *Compare-and-Swap-Schleife*, in der Millionen Threads kollidieren. Reduziert man dagegen in Gruppen vor und passt die Gruppenzahl in Stufe 5 an, erreicht SYCL auf der L40S **99 % von CUB**, und das eingebaute **`sycl::reduction`** liefert über **1.020 GB/s** auf der H100.

**[Klick 1 – Gesamtbild relativ zu CUDA]** Im Gesamtvergleich gegen die 100 %-CUDA-Linie heißt das: Im globalen Speicher erreicht SYCL **81 bis 108 % von CUDA** – die Abstraktion selbst kostet nichts! Rückstände entstehen gezielt im DPC++-Backend bei Local Memory (**69–80 %**) und Float-Atomics (**5–37 %**).

---

### Beantwortung von F1 & F2 und Fazit (2 Min. | Folien 16–17)

**[Folie 16: Beantwortung von F1 & F2]**

Beantworten wir damit meine beiden Forschungsfragen:
- **[Klick 1 – Zu F1]** SYCL stellt einen **abgestuften Baukasten** bereit: Schon Stufe 1 löst Herstellerbindung und doppelte Codebasen mit einem einzigen Binary. Die Stufen 2 bis 5 erlauben es, stufenlos zwischen Komfort (`malloc_shared`, `sycl::reduction`) und expliziter Kontrolle (`malloc_device`, Events, Work-Groups, Local Memory) zu wählen.
- **[Klick 2 – Zu F2]** Muss man die Hardware am Ende doch berücksichtigen? **Für die Korrektheit nein – für hohe Leistung eindeutig ja!** Erstens wegen harter Hardwaregrenzen ($T = 64$), zweitens weil je nach Problemklasse völlig andere Stufen wirken, und drittens weil im Detail die PTX-Codeerzeugung des Compiler-Backends entscheidet – wobei als Grenze gilt, dass hier DPC++ ohne gezieltes Architektur-Flag und mangels Verfügbarkeit ohne AMD-GPU gemessen wurde.

**[Folie 17: Fazit & Ausblick]**

Drei Sätze zum Mitnehmen:
- **[Klick 1]** *„Einmal schreiben, überall ausführen“* hält SYCL für die **funktionale Korrektheit** zu 100 Prozent ein.
- **[Klick 2]** SYCL vereinheitlicht die **Schnittstelle**, nicht die Physik der Hardware – Tuning bleibt Entwickleraufgabe.
- **[Klick 3]** Auf den letzten Metern entscheiden **Compiler und Backends** – weshalb der Vergleich mit **AdaptiveCpp** und Messungen auf einer **AMD MI300A** der spannende nächste Schritt sind.

Vielen Dank – ich freue mich auf eure Fragen!
