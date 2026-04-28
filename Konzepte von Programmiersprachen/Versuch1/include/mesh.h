#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

// Speichert die drei Eckpunkte eines Dreiecks
struct Triangle {
    glm::vec3 v0, v1, v2;
};

class Mesh {
public:
    std::vector<Triangle> triangles;
    
    // Lädt die Datei und gibt true zurück, wenn es geklappt hat
    bool loadOBJ(const std::string& filename);
};