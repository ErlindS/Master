#include <iostream>
#include <chrono>
#include "scene.h"
#include "renderer.h"
#include "image.h"

int main() {
    // 1. Einstellungen
    const int width = 500;
    const int height = 500;

    // 2. Szene laden und konfigurieren (Modell, Kamera, Licht, Bodenebene)
    Scene scene;
    if (!scene.setup("../models/teapot_n_glass.obj", width, height)) {
        return -1;
    }

    Renderer renderer;

    // 3. Rendern mit Badouel (ohne KD-Baum)
    //std::cout << "\nStarte Rendern (Badouel, ohne KD-Baum)..." << std::endl;
    //Image image_badouel(width, height);
    //renderer.setAlgorithm(IntersectionAlgorithm::BADOUEL);
    //renderer.setUseAcceleration(false);
//
    //auto start_b = std::chrono::high_resolution_clock::now();
    //renderer.render(scene.mesh, scene.camera, scene.light, image_badouel);
    //auto end_b = std::chrono::high_resolution_clock::now();
    //auto duration_b = std::chrono::duration_cast<std::chrono::milliseconds>(end_b - start_b);
    //
    //image_badouel.save("output_badouel.ppm");
    //std::cout << "Fertig! 'output_badouel.ppm' erstellt in " << duration_b.count() << " ms." << std::endl;
//
    //// 4. Rendern mit Möller-Trumbore (ohne KD-Baum)
    //std::cout << "\nStarte Rendern (Möller-Trumbore, ohne KD-Baum)..." << std::endl;
    //Image image_moeller(width, height);
    //renderer.setAlgorithm(IntersectionAlgorithm::MOELLER_TRUMBORE);
    //renderer.setUseAcceleration(false);
    //
    //auto start_m = std::chrono::high_resolution_clock::now();
    //renderer.render(scene.mesh, scene.camera, scene.light, image_moeller);
    //auto end_m = std::chrono::high_resolution_clock::now();
    //auto duration_m = std::chrono::duration_cast<std::chrono::milliseconds>(end_m - start_m);
    //
    //image_moeller.save("output_moeller.ppm");
    //std::cout << "Fertig! 'output_moeller.ppm' erstellt in " << duration_m.count() << " ms." << std::endl;

    // 5. Rendern mit Möller-Trumbore (MIT KD-Baum)
    std::cout << "\nStarte Rendern (Möller-Trumbore, MIT KD-Baum)..." << std::endl;
    Image image_moeller_kd(width, height);
    renderer.setAlgorithm(IntersectionAlgorithm::MOELLER_TRUMBORE);
    renderer.setUseAcceleration(true);
    
    auto start_kd = std::chrono::high_resolution_clock::now();
    renderer.render(scene.mesh, scene.camera, scene.light, image_moeller_kd);
    auto end_kd = std::chrono::high_resolution_clock::now();
    auto duration_kd = std::chrono::duration_cast<std::chrono::milliseconds>(end_kd - start_kd);
    
    image_moeller_kd.save("output_moeller_kdtree.ppm");
    std::cout << "Fertig! 'output_moeller_kdtree.ppm' erstellt in " << duration_kd.count() << " ms." << std::endl;

    // 6. Auswertung
    //std::cout << "\n=== ZUSAMMENFASSUNG DER RENDERZEITEN ===" << std::endl;
    //std::cout << "Badouel (ohne KD-Baum):         " << duration_b.count() << " ms" << std::endl;
    //std::cout << "Möller-Trumbore (ohne KD-Baum): " << duration_m.count() << " ms" << std::endl;
    std::cout << "Möller-Trumbore (mit KD-Baum):  " << duration_kd.count() << " ms" << std::endl;
    
    return 0;
}