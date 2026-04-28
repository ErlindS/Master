#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <glm/glm.hpp>
#include <chrono>
#include "camera.h"
#include "intersector.h"
#include "mesh.h"

// Hilfsfunktion: Sucht das nächste getroffene Dreieck (Brute-Force)
bool findClosestHit(const Ray& ray, const std::vector<Triangle>& triangles, Intersection& closestIsect, Triangle& hitTriangle) {
    closestIsect.hit = false;
    closestIsect.t = std::numeric_limits<float>::max();
    bool hitAnything = false;

    for (const auto& tri : triangles) {
        Intersection isect = Intersector::intersectRayTriangle(ray, tri.v0, tri.v1, tri.v2);
        if (isect.hit && isect.t < closestIsect.t) {
            closestIsect = isect;
            hitTriangle = tri;
            hitAnything = true;
        }
    }
    return hitAnything;
}

int main() {
    // 1. Einstellungen
    const int width = 500;
    const int height = 500;
    std::ofstream imageFile("output.ppm");

    // 2. Szene laden
    Mesh scene;
    // WICHTIG: Passe den Pfad hier so an, wo deine Datei vom build-Ordner aus gesehen liegt!
    if (!scene.loadOBJ("../models/teapot_n_glass.obj")) {
        return -1; 
    }

    // 3. Lichtquelle definieren (Von oben rechts, um schöne Schatten nach links zu werfen)
    glm::vec3 lightPos(6.0f, 8.0f, -5.0f);

    // 4. Kamera (Wieder auf der Seite der Tassen, aber etwas zentrierter und leicht von oben)
    Camera cam(glm::vec3(-3.0f, 3.5f, -8.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 45.0f, width, height);

    imageFile << "P3\n" << width << " " << height << "\n255\n";

    // 5. Render Loop
    std::cout << "Starte Rendern..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            float u = (float)x / (width - 1);
            float v = (float)y / (height - 1);

            Ray ray = cam.generateRay(u, v);
            Intersection isect;
            Triangle hitTriangle;

            if (findClosestHit(ray, scene.triangles, isect, hitTriangle)) {
                // --- WIR HABEN ETWAS GETROFFEN: BELEUCHTUNG BERECHNEN ---
                
                // 1. Exakten Punkt im 3D-Raum berechnen (Ursprung + Richtung * Distanz)
                glm::vec3 hitPoint = ray.origin + ray.direction * isect.t;

                // 2. Oberflächennormale berechnen (Wo schaut das Dreieck hin?)
                glm::vec3 edge1 = hitTriangle.v1 - hitTriangle.v0;
                glm::vec3 edge2 = hitTriangle.v2 - hitTriangle.v0;
                glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

                // 3. Vektor vom Trefferpunkt zum Licht
                glm::vec3 lightDir = glm::normalize(lightPos - hitPoint);

                // 4. SCHATTENSTRAHL (Shadow Ray)
                // Wir schieben den Startpunkt minimal in Richtung der Normale (0.001f), 
                // damit das Dreieck sich nicht selbst verdeckt (Shadow Acne)
                float distanceToLight = glm::length(lightPos - hitPoint);
                Ray shadowRay = {hitPoint + normal * 0.001f, lightDir};
                Intersection shadowIsect;
                Triangle dummyTriangle;
                bool inShadow = findClosestHit(shadowRay, scene.triangles, shadowIsect, dummyTriangle);

                // Nur Objekte ZWISCHEN Trefferpunkt und Lichtquelle erzeugen Schatten
                if (inShadow && shadowIsect.t > distanceToLight) {
                    inShadow = false;
                }

                if (inShadow) {
                    // Punkt liegt im Schatten (nur ganz dunkles Umgebungslicht)
                    imageFile << "20 20 20\n"; 
                } else {
                    // 5. Basisfarbe berechnen (Lambert Diffuse)
                    // Je direkter das Licht darauf scheint, desto heller (Skalarprodukt)
                    float diff = std::max(glm::dot(normal, lightDir), 0.0f);
                    
                    // Wir geben der Teekanne eine Basis-Farbe (z.B. Weiß/Grau) und multiplizieren mit dem Licht
                    int colorVal = (int)(255 * diff);
                    imageFile << colorVal << " " << colorVal << " " << colorVal << "\n";
                }
            } else {
                // Hintergrundfarbe (Schwarz)
                imageFile << "0 0 0\n";
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    imageFile.close();
    std::cout << "Fertig! 'output.ppm' wurde erstellt." << std::endl;
    std::cout << "Renderzeit: " << duration_ms.count() << " ms" << std::endl;
    return 0;
}