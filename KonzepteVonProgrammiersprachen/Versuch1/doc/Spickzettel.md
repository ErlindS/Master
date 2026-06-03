# Spickzettel für das Kolloquium: Raytracer & k-d-Baum

Dieses Dokument dient als kompakte Übersicht, in welcher Datei und welcher Methode die wichtigsten Konzepte des Projekts implementiert sind.

## 1. Brute-Force Suche & Schnittpunkte (Aufgabe 1)
*Hier wird berechnet, ob und wo ein Lichtstrahl ein Dreieck trifft, indem alle Dreiecke linear getestet werden.*

* **Datei:** `src/shading.cpp`
* **Brute-Force Suche:** Methode `Shading::findClosestHit()`. Iteriert über **alle** Dreiecke der Szene und ruft für jedes Dreieck die Schnittpunkt-Methode auf (Komplexität $\mathcal{O}(n)$). Speichert den Treffer mit der geringsten Distanz ($t$-Wert).

* **Datei:** `src/intersector.cpp` bzw. `include/intersector.h`
* **Möller-Trumbore-Algorithmus:** Methode `Intersector::intersectRayTriangle()`. Löst das Problem direkt im 3D-Raum mit der Cramerschen Regel, ohne vorher die Ebenengleichung auszurechnen. Ermittelt Parameter $t$, $u$ und $v$.
* **Badouel-Algorithmus:** Methode `Intersector::intersectRayTriangleBadouel()`. Die Basislinie (Brute-Force). Projiziert das Dreieck in 2D (xy-, xz- oder yz-Ebene je nach dominierender Achse) und macht dort den Point-in-Polygon-Test.

## 2. Beschleunigungsdatenstruktur k-d-Baum (Aufgabe 2)
*Hier wird die $\mathcal{O}(n)$ Brute-Force Suche auf den Durchschnittsfall $\mathcal{O}(\log n)$ optimiert.*

* **Datei:** `src/kdtree.cpp` und `include/kdtree.h`
* **Baum-Aufbau (Top-Down, Median):** Methode `KDTree::buildRecursive()`. 
  * Sucht die längste Achse der aktuellen Bounding-Box.
  * Sortiert die Dreiecke nach ihrem Schwerpunkt (Centroid) mit `std::sort`.
  * Teilt die Liste exakt in der Mitte (`count / 2`).
* **Traversierung & Suche:** Methode `KDTree::intersectRecursive()`. Geht durch den Baum.
* **Slab-Test:** Methode `AABB::intersect()`. Prüft super-effizient, ob der Strahl die Bounding-Box trifft. Verwendet `ray.invDirection` (aus `include/ray.h`), um in der Schleife keine langsamen Divisionen machen zu müssen.
* **Early Rejection (Dynamic tMax):** In `intersectRecursive` wird dem Slab-Test der bisher beste gefundene $t$-Wert (`closestIsect.t`) als `tMax` übergeben. Liegt eine Bounding-Box weiter entfernt, bricht der Test ab.

## 3. Beleuchtung & Schatten (Aufgabe 1)
*Hier entstehen die realistischen Farben und Schattierungen.*

* **Datei:** `src/shading.cpp`
* **Shadow Rays (Schattenprüfung):** Methode `Shading::isInShadow()`. Schießt einen sekundären Strahl vom getroffenen Punkt zur Lichtquelle, um zu prüfen, ob ein Objekt dazwischen liegt. Gegen *Shadow Acne* wird der Startpunkt leicht entlang der Normalen verschoben (`normal * 0.001f`). Die Distanzprüfung verhindert, dass Objekte hinter der Lichtquelle Schatten werfen.

* **Datei:** `src/renderer.cpp` (speziell in der Methode `Renderer::traceRay()`)
* **Blinn-Phong Beleuchtungsmodell:** In `traceRay()` aufgeteilt in:
  * *Ambient:* Pauschales Restlicht (15% der Grundfarbe).
  * *Diffuse (Lambert):* Hängt vom Winkel zwischen Normale und Lichtstrahl ab (`glm::dot(normal, lightDir)`).
  * *Specular:* Das Glanzlicht (Halfway-Vektor und Specular-Exponent `Ns`).
* **Baryzentrische Interpolation:** Die Oberflächennormale wird aus den 3 Eckpunkt-Normalen glatt interpoliert (Smooth Shading). Passiert in `traceRay()` mittels den baryzentrischen Koordinaten $u$ und $v$.
* **Double-Sided Shading:** Falls eine Normale von uns wegzeigt (`dot(normal, viewDir) < 0`), drehen wir sie um (`normal = -normal;`), um schwarze Flächen zu vermeiden (z. B. beim Tablett).

## 4. Anti-Aliasing (Kantenglättung)
*Verhindert Treppcheneffekte an den Objektkanten.*

* **Datei:** `src/renderer.cpp` (in der Methode `render()`)
* **Konzept:** *Multisampling/Jittering*. Es werden mehrere Strahlen pro Pixel geschossen, deren $x$- und $y$-Koordinaten minimal zufällig verschoben werden (`std::uniform_real_distribution`). Der Farb-Durchschnitt glättet die Kanten.

## 5. Modell-Import (OBJ-Loader)
*Das Einlesen der 3D-Szene.*

* **Datei:** `src/mesh.cpp` (Methode `Mesh::loadOBJ()`)
* **Was dort passiert:** Parst `.obj` (Geometrie) und `.mtl` (Materialien).
* **Fan-Triangulierung:** Flächen mit mehr als 3 Ecken (Polygone) werden vom Loader dynamisch sternförmig in mehrere Dreiecke zerschnitten.
* **Fallback-Normalen:** Fehlen Vertex-Normalen, rechnet der Loader eine *Flat Shading* Normale über das Kreuzprodukt der Kanten aus.

## 6. Programmstart & Zeitmessung
*Der Einstiegspunkt der Anwendung.*

* **Datei:** `src/main.cpp`
* **Was dort passiert:** Konfiguriert die Szene (Kamera, Lichtposition, Modellpfad) und führt die Render-Loop aus. Nutzt `std::chrono::high_resolution_clock` für präzise Performance-Messungen.

## 7. Kamera & Strahlengenerierung
*Wie aus dem 2D-Bildschirm ein 3D-Blickwinkel wird.*

* **Datei:** `src/camera.cpp` (bzw. Kamera-Logik in `Renderer::render`)
* **Konzept:** Die Bildfläche wird in Pixel unterteilt. Für jeden Pixel werden Bildkoordinaten $(u, v)$ im Bereich $[0, 1]$ generiert. Die Kamera (häufig mit Position, *LookAt*- und *Up*-Vektor definiert) schießt dann einen `Ray` (Primärstrahl) vom Kamera-Ursprung in Richtung dieser Pixel-Koordinate auf einer virtuellen Bildebene (Image Plane).

## 8. Mathematik & Vektorrechnung
*Das algebraische Herz des Raytracers.*

* **Bibliothek:** `GLM` (OpenGL Mathematics) - Header-Only Library.
* **Skalarprodukt (Dot-Product):** Wird exzessiv genutzt, um Winkel zu berechnen. Z.B. für die Ausrichtung der Normalen zum Licht (`dot(normal, lightDir)`), um den Helligkeitswert (Diffuse) zu berechnen.
* **Kreuzprodukt (Cross-Product):** Generiert Vektoren, die orthogonal auf zwei anderen stehen. Wird in `intersectRayTriangle` (Möller-Trumbore) und zur Generierung von *Flat Shading* Fallback-Normalen im OBJ-Loader verwendet.

## 9. Bildausgabe (PPM-Format)
*Wie das fertige Bild auf der Festplatte landet.*

* **Format:** Das `PPM` (Portable Pixmap) Format im `P6` Modus.
* **Warum dieses Format?** Es ist extrem simpel. Es besteht nur aus einem kurzen Text-Header (z.B. `P6\n800 600\n255\n`) gefolgt von unkomprimierten binären RGB-Bytes. Man braucht dafür keine aufwändige externe Bibliothek wie `libpng` oder `libjpeg`.

## 10. Ausblick: Aufgabe 3 (AVX / Packet Tracing)
*Wie Hardware-Parallelisierung den Code weiter beschleunigen wird.*

* **Konzept:** Aktuell rechnet der Raytracer Strahl für Strahl (Skalar). Moderne CPUs haben **SIMD**-Register (Single Instruction, Multiple Data). Mit *AVX2* sind diese Register 256 Bit groß.
* **Ziel:** Wir können $8 \times 32$-Bit Fließkommazahlen (`float`) in ein Register packen. Beim *Packet Tracing* fassen wir 8 Lichtstrahlen zusammen und testen sie in einer einzigen Rechenoperation gleichzeitig gegen die Dreiecke oder den k-d-Baum.

---
**Typische Fragen für die Prüfung:** 
* **"Warum ist der k-d-Baum so viel schneller?"**
  * *"Weil wir im Durchschnitt logarithmische statt lineare Suchzeit haben. Durch den AABB-Slab-Test an inneren Baumknoten und das Setzen von `tMax` (Early Rejection) können wir ganze Geometrie-Gruppen frühzeitig ausschließen, wenn sie nicht getroffen werden."*
* **"Was genau ist Baryzentrische Interpolation?"**
  * *"Ein Koordinatensystem innerhalb eines Dreiecks (Werte $u$, $v$ und $w=1-u-v$). Wir nutzen es, um Vertex-Eigenschaften wie die Flächennormale weich und stufenlos über das gesamte Dreieck zu überblenden (Smooth Shading)."*
* **"Wo machen Sie eigentlich das Anti-Aliasing (Kantenglättung)?"**
  * *Zeigen in:* `src/renderer.cpp` in `Renderer::render()` (Zeile 119+). Hier werden $N$ Strahlen pro Pixel geschossen und über `std::uniform_real_distribution` minimal auf der Bildachse gejittert.
* **"Wo interpolieren Sie die Normalen für das Smooth Shading?"**
  * *Zeigen in:* `src/renderer.cpp` (`Renderer::traceRay()`, ca. Zeile 45). Das geschieht mit `float w = 1.0f - isect.u - isect.v;` unter Verwendung der baryzentrischen Koordinaten aus dem Möller-Trumbore-Algorithmus.
* **"Was passiert, wenn wir ein Objekt von hinten betrachten? Wo fangen Sie das ab?"**
  * *Zeigen in:* `src/renderer.cpp` (ca. Zeile 51). Durch das *Double-Sided Shading* (`if (glm::dot(normal, viewDir) < 0.0f) normal = -normal;`). Es dreht wegzeigende Normalen zur Kamera hin.
* **"Wo und wie verfeinern Sie die Baumtraversierung (Early Rejection beim k-d-Baum)?"**
  * *Zeigen in:* `src/kdtree.cpp` (`KDTree::intersectRecursive()`, Zeile 118). Nach dem Besuch des vorderen Kindknotens prüfen wir: `if (tMinR < closestIsect.t)`. Ist die Distanz zur Bounding-Box des verbleibenden Knotens größer als unser bisher bester Treffer (`closestIsect.t`), überspringen wir diesen komplett.
* **"Wo wird das Problem der Shadow Acne behoben?"**
  * *Zeigen in:* Bei der Initialisierung des Schattenstrahls (`Ray shadowRay(hitPoint + normal * 0.001f, lightDir);` in `src/renderer.cpp` oder `shading.cpp`). Der kleine Offset schiebt den Startpunkt entlang der Normalen von der Oberfläche weg.
