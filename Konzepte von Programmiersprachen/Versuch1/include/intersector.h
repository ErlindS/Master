#pragma once
#include "ray.h"
#include <glm/glm.hpp>

// Speichert das Ergebnis eines Schnittpunkttests
struct Intersection {
    bool hit;          // Wurde das Dreieck getroffen?
    float t;           // Distanz auf dem Strahl (Ray)
    float u, v;        // Baryzentrische Koordinaten auf dem Dreieck
};

class Intersector {
public:
    // Führt den Möller-Trumbore Test für einen Strahl und drei Eckpunkte durch
    static Intersection intersectRayTriangle(
        const Ray& ray, 
        const glm::vec3& v0, 
        const glm::vec3& v1, 
        const glm::vec3& v2
    );
};