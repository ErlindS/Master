#include <iostream>
#include <chrono>
#include <string>
#include "scene.h"
#include "camera.h"
#include "renderer.h"
#include "obj_loader.h"

int main(int argc, char* argv[]) {
    // --- Standardwerte ---
    int         width      = 800;
    int         height     = 600;
    std::string obj        = "../examples/teapot_n_glass.obj";
    std::string output     = "output.ppm";
    bool        use_packet = false;
    bool        use_brute  = false;  // -brute: kein k-d-Baum (Aufgabe 1)
    bool        use_mt     = true;   // -algo naive: Badouel statt Möller-Trumbore

    // --- Argument-Parsing ---
    for (int i = 1; i < argc; ++i) {
        std::string a(argv[i]);
        if (a == "-width"  && i+1 < argc) width  = std::stoi(argv[++i]);
        if (a == "-height" && i+1 < argc) height = std::stoi(argv[++i]);
        if (a == "-output" && i+1 < argc) output = argv[++i];
        if (a == "-obj"    && i+1 < argc) obj    = argv[++i];
        if (a == "-packet") use_packet = true;
        if (a == "-brute")  use_brute  = true;
        if (a == "-algo" && i+1 < argc) {
            std::string algo(argv[++i]);
            use_mt = (algo != "naive");
        }
    }

    std::string modus;
    if (use_brute)
        modus = use_mt ? "Brute-Force + Möller-Trumbore" : "Brute-Force + Badouel";
    else if (use_packet)
        modus = "Packet (8 Strahlen/AVX2)";
    else
        modus = "k-d-Baum (skalär)";

#ifdef HAVE_AVX2
    std::cout << "=== Whitted-Style Raytracer -- Aufgabe 3 (AVX2 Packet Tracing) ===\n";
#else
    if (use_brute)
        std::cout << "=== Whitted-Style Raytracer -- Aufgabe 1 (Brute-Force) ===\n";
    else
        std::cout << "=== Whitted-Style Raytracer -- Aufgabe 2 (k-d-Baum) ===\n";
#endif
    std::cout << "Auflösung : " << width << " x " << height << "\n";
    std::cout << "OBJ-Datei : " << obj << "\n";
    std::cout << "Ausgabe   : " << output << "\n";
    std::cout << "Modus     : " << modus << "\n\n";

    // --- Szene laden ---
    Scene scene;
    if (!load_obj(obj, scene)) return 1;

    // --- k-d-Baum bauen (nur wenn nicht Brute-Force) ---
    if (!use_brute) {
        scene.kd.build(scene.triangles);
        std::cout << "k-d-Baum  : " << scene.kd.nodes.size() << " Knoten, "
                  << scene.triangles.size() << " Dreiecke\n";
    } else {
        std::cout << "Dreiecke  : " << scene.triangles.size() << " (kein k-d-Baum)\n";
    }
    std::cout << "\n";

    // --- Lichtquellen (einmalig festlegen, nie mehr ändern!) ---
    scene.lights.push_back({{ 3.f,  6.f,  4.f}, {1.0f, 1.0f, 1.0f}, 1.0f});
    scene.lights.push_back({{-3.f,  4.f, -2.f}, {0.7f, 0.8f, 1.0f}, 0.6f});

    // --- Kamera (einmalig festlegen, nie mehr ändern!) ---
    Camera cam(
        { 0.0f, 2.5f,-9.f},   // Auge: frontal von vorne (Tassen bei z≈-2 liegen VOR Teekanne z≈+1)
        { 0.0f, 1.0f, 0.f},   // Blickziel: Mitte Szene
        { 0.0f, 1.0f, 0.f},   // Oben-Vektor
        45.f,                  // Sichtfeld
        (float)width / height
    );

    // --- Rendern mit Zeitmessung ---
    Image img(width, height);
    auto  t0 = std::chrono::high_resolution_clock::now();

#ifdef HAVE_AVX2
    if (use_packet)
        render_packet(scene, cam, img);
    else
#endif
    if (use_brute)
        render_brute(scene, cam, img, use_mt);
    else
        render(scene, cam, img);

    auto      t1 = std::chrono::high_resolution_clock::now();
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "Laufzeit  : " << ms << " ms\n";

    // --- Bild speichern ---
    if (!img.save_ppm(output)) return 1;
    std::cout << "Fertig! Öffne '" << output << "' in Preview.\n";
    return 0;
}
