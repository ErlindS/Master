# Projektübersicht: Raytracer

Dieses Dokument bietet eine vollständige Übersicht über die Architektur des C++-Raytracers. Die Codebasis ist in Header-Dateien (`include/`) und die zugehörigen Implementierungen (`src/`) aufgeteilt.

## Kernstrukturen & Datenmodelle

### 1. `ray.h`
Definiert den elementaren Lichtstrahl für das Raytracing.
*   **`struct Ray`**: 
    *   Speichert den Ursprung (`origin`) und die Richtung (`direction`).
    *   Speichert zusätzlich die invertierte Richtung (`invDirection = 1.0 / direction`), was eine Optimierung darstellt, um im BVH Slab-Test teure Divisionen durch schnelle Multiplikationen zu ersetzen.

### 2. `image.h` & `src/image.cpp`
Zuständig für die Speicherung und Ausgabe des gerenderten Bildes.
*   **`class Image`**: 
    *   Hält die Auflösung (`width`, `height`) und ein Array aller Pixel-Farben.
    *   `setPixel()`: Schreibt die berechnete Farbe (als `glm::vec3`) in einen bestimmten Pixel.
    *   `save()`: Exportiert das Bild im `.ppm`-Format auf die Festplatte.

### 3. `mesh.h` & `src/mesh.cpp`
Verwaltet die Geometrie der Szene, also alle zu rendernden Objekte.
*   **`struct Triangle`**: Speichert die 3 Eckpunkte (`v0, v1, v2`), die Normalen für glattes Shading, sowie Materialeigenschaften (Diffuse Farbe `Kd`, Specular Farbe `Ks`, Glanz-Exponent `Ns`).
*   **`class Mesh`**: 
    *   `loadOBJ()`: Parst Wavefront `.obj` und zugehörige `.mtl` Dateien, um Dreiecke und Materialien zu laden.
    *   `addGroundPlane()`: Generiert prozedural zwei Dreiecke, die als Boden fungieren, damit Schatten sichtbar werden.
    *   Verwaltet intern die Beschleunigungsdatenstruktur (`BVH`).

## Algorithmen & Logik

### 4. `intersector.h` & `src/intersector.cpp`
Enthält die eigentliche Mathematik, um zu testen, ob ein Strahl ein Dreieck trifft.
*   **`struct Intersection`**: Speichert, ob es einen Treffer gab (`hit`), die Distanz auf dem Strahl (`t`) sowie die baryzentrischen Koordinaten (`u, v`) auf dem Dreieck.
*   **`class Intersector`**:
    *   `intersectRayTriangle()`: Implementiert den optimierten **Möller-Trumbore**-Schnittpunkttest (aus Aufgabe 1).
    *   `intersectRayTriangleBadouel()`: Implementiert **Badouels Algorithmus** (aus der Vorlesung).

### 5. `bvh.h` & `src/bvh.cpp`
Die Beschleunigungsdatenstruktur (Bounding Volume Hierarchy) aus **Aufgabe 2**, um die Anzahl der Schnittpunkttests drastisch zu reduzieren.
*   **`struct AABB`**: Repräsentiert eine Bounding Box. Enthält die Funktion `intersect()`, die den **Slab-Test** ausführt, um extrem schnell zu prüfen, ob ein Strahl die Box streift.
*   **`struct BVHNode`**: Ein Knoten im Baum. Speichert die Bounding Box, Zeiger auf linkes/rechtes Kind (wenn innerer Knoten) oder Indizes zu den Dreiecken (wenn Blatt-Knoten).
*   **`class BVH`**:
    *   `buildRecursive()`: Baut den Baum auf, indem Dreiecke entlang der längsten Achse der Bounding Box sortiert und genau in der Mitte geteilt werden (**Median-Split**).
    *   `intersectRecursive()`: Traversiert den Baum beim Rendern. Wenn der Strahl die Bounding-Box trifft (Slab-Test), wird in den Kindern weitergesucht, andernfalls wird der Ast übersprungen.

### 6. `shading.h` & `src/shading.cpp`
Zuständig für Licht und Schatten (Beleuchtungsmodell).
*   **`struct Light`**: Definiert eine einfache Punktlichtquelle im Raum.
*   **`class Shading`**:
    *   `isInShadow()`: Schießt einen Schattenstrahl (Shadow Ray) vom getroffenen Oberflächenpunkt zur Lichtquelle. Wenn ein Objekt im Weg ist, liegt der Punkt im Schatten.
    *   `computeShading()`: Berechnet das Phong-Beleuchtungsmodell (Ambient + Diffuse + Specular) für einen Pixel unter Berücksichtigung von Schatten.

## Zusammenbau & Steuerung

### 7. `camera.h`
Modelliert das Auge/die virtuelle Kamera im Raum.
*   **`class Camera`**: 
    *   Konfiguriert das Sichtfeld (FOV), Position (`origin`), Blickrichtung (`lookAt`) und das Up-Vektor-System (`up`).
    *   `generateRay(s, t)`: Erschafft für jede relative 2D-Pixelkoordinate $(s, t)$ auf der Bildebene den entsprechenden 3D-Lichtstrahl in die Szene.

### 8. `scene.h` & `src/scene.cpp`
Bündelt alle Komponenten der Welt an einem Ort.
*   **`struct Scene`**: 
    *   Enthält eine Instanz von `Mesh`, `Camera` und `Light`.
    *   `setup()`: Initialisiert die komplette Umgebung, lädt das Modell, platziert das Licht und bereitet die Kamera vor.

### 9. `renderer.h` & `src/renderer.cpp`
Der Kern des Raytracers. Verknüpft Szene, Kamera, BVH und Bild.
*   **`class Renderer`**:
    *   `render()`: Enthält die Hauptschleife über alle Bild-Pixel (x, y). Generiert über die Kamera einen Strahl (`traceRay()`).
    *   `findClosestHit()`: Nutzt die BVH, um das erste Dreieck zu finden, das der Strahl trifft.
    *   Übergibt den Treffer an `Shading::computeShading()` und speichert die resultierende Farbe im Bild.

### 10. `main.cpp`
Der Einsprungpunkt (Entry Point) des Programms.
*   Konfiguriert die Auflösung und startet den `Scene::setup()`.
*   Rendert das gleiche Bild zwei Mal (einmal mit Möller-Trumbore, einmal mit Badouel) zur Zeitmessung.
*   Speichert die Bilder ab und gibt die gemessenen Ausführungszeiten auf der Konsole aus.
