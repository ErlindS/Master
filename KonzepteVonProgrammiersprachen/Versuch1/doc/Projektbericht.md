# Laborbericht: Konzepte von Programmiersprachen – Raytracer
**Name:** [Name hier einfügen]
**Datum:** 28.04.2026

## 1. Implementierung eines Whitted-Style Raytracers

Für die erste Rechneraufgabe wurde ein rudimentärer Whitted-Style Raytracer in C++ implementiert. Die wesentliche Aufgabe bestand in der Berechnung der Schnittpunkte zwischen den generierten Primärstrahlen und der Szenengeometrie. Zur Effizienzsteigerung der Schnittpunktberechnung wurde zunächst als Baseline der Schnittpunkttest nach Badouel implementiert. Dieser Algorithmus berechnet zunächst den 3D-Schnittpunkt mit der Dreiecksebene und projiziert die Koordinaten anschließend auf eine 2D-Ebene (durch Weglassen der dominierenden Normalenachse), um den Point-in-Polygon-Test durch Lösen eines linearen 2D-Gleichungssystems effizienter zu gestalten.

Als erste Optimierungsmaßnahme gegenüber dieser Baseline wurde zusätzlich der Möller-Trumbore-Algorithmus umgesetzt. Dieser Algorithmus bietet eine performantere Alternative, indem er die Berechnung direkt auf Basis baryzentrischer Koordinaten ohne explizite Konstruktion der Dreiecksebenengleichung oder Fallunterscheidungen durchführt.

Zur Repräsentation der Testszene wurde eine einfache Import-Funktion für das Wavefront-Format (`.obj`) integriert. Diese liest die bereitgestellte Testszene in eine flache, lineare Datenstruktur (Liste von Dreiecken) ein. Zum aktuellen Zeitpunkt (Aufgabe 1) erfolgt die Bestimmung des nächstgelegenen Schnittpunktes durch eine vollständige Brute-Force-Suche über alle eingelesenen Dreiecke für jeden abgefeuerten Strahl, da noch keine Beschleunigungsdatenstruktur wie ein k-d-Baum vorliegt.

Um einen grundlegenden Eindruck von Plastizität zu erzeugen, wurde eine punktförmige Lichtquelle implementiert. Ausgehend von einem ermittelten Schnittpunkt auf der Geometrie wird für jeden Bildpunkt ein zusätzlicher Schattenstrahl in Richtung der Lichtquelle ausgesendet. Trifft dieser Schattenstrahl auf seinem Weg ein weiteres Geometrieobjekt, wird der aktuelle Bildpunkt als verschattet deklariert und lediglich mit einem schwachen Umgebungslast (Ambient Light) beleuchtet. Andernfalls erfolgt die Färbung nach einem vereinfachten Lambert-Beleuchtungsmodell, bei dem der Einfallswinkel des Lichts auf die Oberflächennormale berücksichtigt wird.

Die wesentliche Implementierung der Schnittpunktberechnung im Code stellt sich wie folgt dar:

```cpp
Intersection Intersector::intersectRayTriangle(
    const Ray& ray, 
    const glm::vec3& v0, 
    const glm::vec3& v1, 
    const glm::vec3& v2) 
{
    Intersection result = {false, 0.0f, 0.0f, 0.0f};

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(ray.direction, edge2);
    float a = glm::dot(edge1, h);

    constexpr float EPSILON = 0.0000001f;
    if (a > -EPSILON && a < EPSILON) {
        return result; 
    }

    float f = 1.0f / a;
    glm::vec3 s = ray.origin - v0;
    result.u = f * glm::dot(s, h);
    if (result.u < 0.0f || result.u > 1.0f) {
        return result;
    }

    glm::vec3 q = glm::cross(s, edge1);
    result.v = f * glm::dot(ray.direction, q);
    if (result.v < 0.0f || result.u + result.v > 1.0f) {
        return result;
    }

    result.t = f * glm::dot(edge2, q);
    if (result.t > EPSILON) {
        result.hit = true;
        return result;
    }

    return result; 
}
```

### 1.1 Zeitmessung

Die Zeitmessung der Ausführung des Renderers dient als Basislinie für spätere Optimierungsaufgaben. Gemessen wurde hierbei ausschließlich die Dauer des Render-Loops für die Szene `teapot_n_glass.obj` (18.032 Dreiecke) bei einer festgelegten Auflösung von 500x500 Bildpunkten inklusive der Berechnung harter Schatten. Der C++-Code wurde mittels `g++` über CMake unter Verwendung des `-O3`-Flags im Release-Modus übersetzt.

| Implementierungsschritt              | Badouel [ms] | Möller-Trumbore [ms] |
|:-------------------------------------|-------------:|---------------------:|
| Aufgabe 1: Brute-Force (ohne kd-Baum)|       100067 |                65019 |

Aus den Messergebnissen ist deutlich ersichtlich, dass die lineare Brute-Force-Suche bei komplexen Szenen schnell ineffizient wird, was die Notwendigkeit für die nachfolgenden Aufgaben bezüglich räumlicher Datenstrukturen und Hardware-Parallelisierung unterstreicht.

## 2. Reduzierung der Schnittpunkttests (Ausblick Aufgabe 2)
*(Dieser Abschnitt wird ausgefüllt, sobald der k-d-Baum mit Slab-Test implementiert wurde.)*

## 3. Optimierung mit AVX-Erweiterungen (Ausblick Aufgabe 3)
*(Dieser Abschnitt wird ausgefüllt, sobald das Packet-Tracing implementiert ist.)*
