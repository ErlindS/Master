#include "scene.h"

bool Scene::setup(const std::string& modelPath, int width, int height) {
    // 1. Modell laden
    if (!mesh.loadOBJ(modelPath)) {
        return false;
    }

    // 2. Bodenebene hinzufügen (für sichtbare Schatten)
    //mesh.addGroundPlane(-0.01f, 15.0f, glm::vec3(0.6f, 0.6f, 0.6f));

    // 3. Lichtquelle definieren (Von oben rechts, um schöne Schatten nach links zu werfen)
    light = Light(glm::vec3(6.0f, 8.0f, -5.0f));

    // 4. Kamera (auf der Seite der Tassen, etwas zentriert und leicht von oben)
    camera = Camera(
        glm::vec3(-3.0f, 3.5f, -8.0f),   // Position
        glm::vec3(0.0f, 1.0f, 0.0f),      // Blickpunkt
        glm::vec3(0.0f, 1.0f, 0.0f),      // Up-Vektor
        45.0f, width, height
    );

    return true;
}
