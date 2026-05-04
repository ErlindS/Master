#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Triangle {
    glm::vec3 v0, v1, v2;
    glm::vec3 n0, n1, n2; // Vertex-Normalen für die Interpolation
    glm::vec3 color;      // Materialfarbe (Diffuse Kd)
    glm::vec3 specularColor; // Glanzfarbe (Specular Ks)
    float specularExponent;  // Glanz-Exponent (Shininess Ns)
};

class Mesh {
public:
    std::vector<Triangle> triangles;
    
    // Lädt die Datei und gibt true zurück, wenn es geklappt hat
    bool loadOBJ(const std::string& filename);
};