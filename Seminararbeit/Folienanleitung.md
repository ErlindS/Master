# Dramaturgie & Folienkonzept: SYCL (Überarbeitete Neufassung)

> **Dateien im Überblick:**
> - **Fertige HTML-Präsentation:** [sycl_praesentation.html](file:///home/debian/repo/Master/Seminararbeit/Praesentation/sycl_praesentation.html) *(inkl. Dual-Screen-Referentenansicht auf Taste `P`)*
> - **Synchronisierter Vortragstext:** [Sprechtext.md](file:///home/debian/repo/Master/Seminararbeit/Sprechtext.md)

---

## Was gegenüber einer starren Vorlage grundlegend verbessert wurde

1. **Kein „Betreutes Lesen“ und keine 6er-Kleinkariert-Kästchen beim Einstieg (Folie 2):**
   Statt 6 Mini-Boxen gleichzeitig auf einer Folie zu zeigen (was die Pointe vorwegnimmt), nutzt Folie 2 eine **Vollbild-Bühne (Cinema Storyboard)**: Jeder Klick wechselt die Szene groß in der Mitte, bis Klick 6 in einem echten CUDA/nvcc-Terminal-Abbruch auf AMD gipfelt.
2. **„Das Bild wird repariert“ als echtes Live-Morphing + 3. Kernproblem ergänzt (Folie 3):**
   Statt zwei getrennte Diagramme untereinander zu quetschen, **repariert sich ein einziges großes Architektur-Schaubild per Klick live vor den Augen des Publikums** (`programm.cu` + rote `✕`-Pfade $\longrightarrow$ `SYCL: AdaptiveCpp / DPC++` + 4 geheilte Pfade). Zusätzlich ergänzt Klick 2 das dritte Kernproblem aus deiner Seminararbeit (*Abstraktion vs. Kontrolle*: OpenMP vs. CUDA/OpenCL vs. SYCL), damit Forschungsfrage F1 sauber motiviert ist.
3. **Methodik erklärt *warum* genau diese 3 Beispiele gewählt wurden (Folie 5):**
   Nicht einfach „drei Beispiele“, sondern **drei gegensätzliche Hardware-Flaschenhälse** (*Rechengebunden* vs. *Speichergebunden* vs. *Synchronisationsgebunden*), auf die derselbe 5-stufige SYCL-Werkzeugkasten losgelassen wird.
4. **Live-Code-Transformation von C++ nach SYCL (Folie 7):**
   Man sieht zuerst die 3 vertrauten `for`-Schleifen aus Stufe 0 – und beim Klick verwandeln sich genau die zwei äußeren Schleifen in `q.parallel_for` (4,2 Mio. Work-Items bei $n=2048$). So versteht jeder Zuhörer Single-Source-SYCL in 5 Sekunden.
5. **Entzerrte Ergebnis-Story mit Transfer-Einsicht & echtem PTX-Beweis (Folien 12–15):**
   Statt 38 Balken auf zwei Folien zu quetschen, hat jede der **4 Kernzahlen** ihre eigene klare Bühne:
   - **Folie 12 (Matmul & Transfer):** Zeigt den Gerätedissens (Stufe 3 gewinnt auf RTX/L40S, Stufe 4 auf H100/CPU) **plus** per Taste `T` deine starke Erkenntnis aus Tabelle 4: Je schneller die GPU rechnet, desto teurer wird PCIe-Transfer (auf der H100 bereits **74 % der Kernelzeit**!).
   - **Folie 13 (Kachelgröße & PP):** Dedizierter Vergleich aller 5 Einträge (`T = 8, 16, 32, 64` + `Stufe 5 auto`) inkl. Erklärung der Spec-Constant-Emulation auf CUDA vs. SPIR-V auf der CPU.
   - **Folie 14 (2D-Stencil):** Kontrastiert direkt `−74 %` Bandbreitenverlust durch `malloc_shared` mit `1.931 GB/s = 101 % von CUDA` durch eigene Work-Group `W = 16`.
   - **Folie 15 (Reduktion & PTX-Mikroskop + CUDA-Gesamtbild):** Zeigt den **Faktor 20.500×** (`39 s` vs. `1,9 ms`) direkt neben den **2 Zeilen PTX-Assembler** (`atom.global.add.f32` vs. Compare-and-Swap-Schleife `atom.relaxed.gpu.global.cas.b32`) – und blendet per Klick das große Gesamtdiagramm relativ zu CUDA (`100 %`) ein.
6. **Echtes Referentenfenster für 2 Bildschirme (`Taste P`):**
   Auf dem Beamer erscheint eine ruhige, edle Folienbühne ohne ablenkende Menüleiste; auf deinem Laptop zeigt das Popup-Fenster Uhrzeit, Soll-Zeitfenster, nächste Folie und den passend zum Klick hervorgehobenen Sprechtext.

---

## Ablaufplan der 17 Folien (20:00 Minuten)

| Folie | Zeitfenster | Titel & Visuelle Mechanik | Klicks |
| :--- | :--- | :--- | :---: |
| **1** | 00:00–00:20 | **Titelfolie:** *SYCL – Ein C++-Quellcode für jede Hardware?* mit Konvergenz-Grafik (`main.cpp` $\rightarrow$ 4 Architekturen) | 0 |
| **2** | 00:20–01:15 | **Storyboard (Vendor Lock-in):** Vollbild-Szenenwechsel: Entwickler $\rightarrow$ Lederjacke $\rightarrow$ CUDA-Stack $\rightarrow$ Preisschock $\rightarrow$ AMD-Karte $\rightarrow$ Terminal-Fehler | 5 |
| **3** | 01:15–02:15 | **Was ist SYCL?** Live-Reparatur des gebrochenen Diagramms + Einordnung *Abstraktion vs. Kontrolle* | 2 |
| **4** | 02:15–04:00 | **Forschungsfragen F1 & F2:** Visuelle Ungleichung *Funktional portabel $\neq$ Performance-portabel*, darunter F1 & F2 | 2 |
| **5** | 04:00–05:00 | **Methodik 1:** 3 animierte Engpässe (*Rechen-*, *Speicher-*, *Sync-gebunden*) + aufsteigende 5-Stufen-Skala | 1 |
| **6** | 05:00–06:00 | **Methodik 2:** Skalierung von Ryzen 5600X & RTX 5060 Ti bis DACHS-Cluster (L40S & H100 80 GB) · 1 Binary (`ONEAPI_DEVICE_SELECTOR`) | 0 |
| **7** | 06:00–07:40 | **Stufe 1:** Host–Queue–Gerät-Modell + **Live-Code-Umbau** von 3 `for`-Schleifen zu `q.parallel_for` (4,2 Mio. Work-Items) | 1 |
| **8** | 07:40–09:20 | **Stufe 2 (& 2b):** 3-Wege-Speichervergleich (`malloc_shared` vs. `malloc_device` vs. `buffer`/`accessor`) + Code-Toggle (`T`) | 0 |
| **9** | 09:20–10:55 | **Stufe 3:** Paralleler Transfer-DAG mit `depends_on` (links) + zweistufiger Indexraum `nd_range` (rechts) | 0 |
| **10** | 10:55–12:35 | **Stufe 4:** Animiertes Work-Group-Tiling (`1. Laden` $\rightarrow$ `2. group_barrier` $\rightarrow$ `3. Lokal rechnen`) + Kernelcode | 0 |
| **11** | 12:35–14:00 | **Stufe 5:** GPU-Hardwarelimit (`T = 64` $\rightarrow$ `4096 > 1024` Crash) vs. Laufzeit-Filterung per `get_info` & *Specialization Constant* | 0 |
| **12** | 14:00–15:00 | **Ergebnis 1 (Matmul):** Bis zu `15.600×` Speedup, unterschiedlicher Stufen-Sieger je Gerät + **Transfer-Anteil (`74 %` auf H100!)** per `T` | 0 |
| **13** | 15:00–15:55 | **Ergebnis 2 (PP):** `PP = 0,00` bei `T = 64` vs. `0,97` bei `T = 16` · Warum Stufe 5 auf CUDA 5–43 % kostet (Spec-Const-Emulation) | 0 |
| **14** | 15:55–16:50 | **Ergebnis 3 (Stencil):** `−74 %` Verlust durch `malloc_shared` auf H100 vs. **`1.931 GB/s = 101 % CUDA`** durch `W = 16` in Stufe 3 | 0 |
| **15** | 16:50–18:00 | **Ergebnis 4 (Reduktion & Gesamtbild):** **`20.500×`** durch CAS-Schleife im **PTX-Code** $\rightarrow$ Klick 1 blendet Gesamtvergleich gegen `100 % CUDA` ein | 1 |
| **16** | 18:00–19:15 | **Beantwortung F1 & F2:** Gestufter Baukasten (F1) vs. 3 reale Hardware-Abhängigkeiten (F2) + transparente Methodengrenzen | 2 |
| **17** | 19:15–20:00 | **Fazit & Ausblick:** 3 prägnante Kernsätze + Ausblick (*AdaptiveCpp* & *AMD MI300A*) | 3 |
