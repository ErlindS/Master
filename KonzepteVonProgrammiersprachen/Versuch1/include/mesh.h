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

#include <memory>

class BVH;

class Mesh {
public:
    std::vector<Triangle> triangles;
    std::unique_ptr<BVH> bvh;
    
    Mesh();
    ~Mesh();
    
    // Lädt die Datei und gibt true zurück, wenn es geklappt hat
    bool loadOBJ(const std::string& filename);

    // Fügt eine Bodenebene (Ground Plane) hinzu, damit Schatten sichtbar werden
    // y: Höhe der Ebene, size: halbe Kantenlänge, color: Farbe der Ebene
    void addGroundPlane(float y, float size, const glm::vec3& color);
};