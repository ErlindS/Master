#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <glm/glm.hpp>
#include <chrono>
#include "camera.h"
#include "intersector.h"
#include "mesh.h"
#include "renderer.h"
#include "image.h"

int main() {
    // 1. Einstellungen
    const int width = 500;
    const int height = 500;

    // 2. Szene laden
    Mesh scene;
    if (!scene.loadOBJ("../models/teapot_n_glass.obj")) {
        return -1; 
    }



    // 3. Lichtquelle definieren (Von oben rechts, um schöne Schatten nach links zu werfen)
    glm::vec3 lightPos(6.0f, 8.0f, -5.0f);

    // 4. Kamera (Wieder auf der Seite der Tassen, aber etwas zentrierter und leicht von oben)
    Camera cam(glm::vec3(-3.0f, 3.5f, -8.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 45.0f, width, height);

    // 5. Rendern mit Möller-Trumbore
    std::cout << "\nStarte Rendern (Möller-Trumbore)..." << std::endl;
    Image image_moeller(width, height);
    Renderer renderer;
    renderer.setAlgorithm(IntersectionAlgorithm::MOELLER_TRUMBORE);
    
    auto start_m = std::chrono::high_resolution_clock::now();
    renderer.render(scene, cam, lightPos, image_moeller);
    auto end_m = std::chrono::high_resolution_clock::now();
    auto duration_m = std::chrono::duration_cast<std::chrono::milliseconds>(end_m - start_m);
    
    image_moeller.save("output_moeller.ppm");
    std::cout << "Fertig! 'output_moeller.ppm' erstellt in " << duration_m.count() << " ms." << std::endl;

    // 6. Rendern mit Badouel
    std::cout << "\nStarte Rendern (Badouel)..." << std::endl;
    Image image_badouel(width, height);
    renderer.setAlgorithm(IntersectionAlgorithm::BADOUEL);

    auto start_b = std::chrono::high_resolution_clock::now();
    renderer.render(scene, cam, lightPos, image_badouel);
    auto end_b = std::chrono::high_resolution_clock::now();
    auto duration_b = std::chrono::duration_cast<std::chrono::milliseconds>(end_b - start_b);
    
    image_badouel.save("output_badouel.ppm");
    std::cout << "Fertig! 'output_badouel.ppm' erstellt in " << duration_b.count() << " ms." << std::endl;

    // 7. Auswertung
    std::cout << "\n=== ZUSAMMENFASSUNG DER RENDERZEITEN ===" << std::endl;
    std::cout << "Möller-Trumbore: " << duration_m.count() << " ms" << std::endl;
    std::cout << "Badouel:         " << duration_b.count() << " ms" << std::endl;
    
    return 0;
}