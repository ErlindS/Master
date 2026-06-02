#pragma once

#include "ray.h"
#include "mesh.h"
#include "intersector.h"
#include <glm/glm.hpp>
#include <vector>

// Repräsentiert eine Punktlichtquelle in der Szene
struct Light {
    glm::vec3 position;  // Position der Lichtquelle im Weltkoordinatensystem

    Light() : position(0.0f) {}
    Light(const glm::vec3& pos) : position(pos) {}
};

// Statische Hilfsklasse für alle Beleuchtungsberechnungen
class Shading {
public:
    // Prüft, ob ein Punkt im Schatten liegt (Shadow Ray)
    static bool isInShadow(
        const glm::vec3& hitPoint,
        const glm::vec3& normal,
        const glm::vec3& lightPos,
        const std::vector<Triangle>& triangles,
        IntersectionAlgorithm algorithm
    );

    // Berechnet die vollständige Beleuchtung (Ambient + Diffuse + Specular)
    static glm::vec3 computeShading(
        const glm::vec3& hitPoint,
        const glm::vec3& normal,
        const glm::vec3& viewDir,
        const Triangle& hitTriangle,
        const Light& light,
        const std::vector<Triangle>& triangles,
        IntersectionAlgorithm algorithm
    );

private:
    // Sucht den nächsten Schnittpunkt entlang eines Strahls
    static bool findClosestHit(
        const Ray& ray,
        const std::vector<Triangle>& triangles,
        Intersection& closestIsect,
        Triangle& hitTriangle,
        IntersectionAlgorithm algorithm
    );
};
