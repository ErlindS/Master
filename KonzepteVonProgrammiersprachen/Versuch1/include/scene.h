#pragma once

#include "mesh.h"
#include "camera.h"
#include "shading.h"
#include <glm/glm.hpp>

// Bündelt alle Szenen-Parameter: Geometrie, Kamera und Beleuchtung
struct Scene {
    Mesh mesh;
    Camera camera;
    Light light;

    // Standardkonstruktor mit Dummy-Kamera (wird in setup() überschrieben)
    Scene() : camera(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,1,0), 45.0f, 1, 1) {}

    // Lädt das Modell, konfiguriert Kamera und Licht, fügt Bodenebene hinzu
    bool setup(const std::string& modelPath, int width, int height);
};
