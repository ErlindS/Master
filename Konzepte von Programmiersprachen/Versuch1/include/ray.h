#pragma once
#include <glm/glm.hpp>

// Repräsentiert einen Sehstrahl
struct Ray {
    glm::vec3 origin;    // Ursprung (z.B. Kameraposition)
    glm::vec3 direction; // Richtung (muss normalisiert sein!)
};