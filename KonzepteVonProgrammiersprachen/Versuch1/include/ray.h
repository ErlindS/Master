#pragma once
#include <glm/glm.hpp>

// Repräsentiert einen Sehstrahl
struct Ray {
    glm::vec3 origin;    // Ursprung (z.B. Kameraposition)
    glm::vec3 direction; // Richtung (muss normalisiert sein!)
    glm::vec3 invDirection; // 1.0 / direction für schnellen Slab-Test

    Ray() = default;
    Ray(const glm::vec3& o, const glm::vec3& d) 
        : origin(o), direction(d), invDirection(1.0f / d) {}
};