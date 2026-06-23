#pragma once
#include "ray.h"
#include "packet.h"
#include <glm/glm.hpp>

// Speichert das Ergebnis eines Schnittpunkttests
struct Intersection {
    bool hit;          // Wurde das Dreieck getroffen?
    float t;           // Distanz auf dem Strahl (Ray)
    float u, v;        // Baryzentrische Koordinaten auf dem Dreieck
};

enum class IntersectionAlgorithm {
    MOELLER_TRUMBORE,
    BADOUEL
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

    // Führt den Badouel Test für einen Strahl und drei Eckpunkte durch
    static Intersection intersectRayTriangleBadouel(
        const Ray& ray, 
        const glm::vec3& v0, 
        const glm::vec3& v1, 
        const glm::vec3& v2
    );

    static IntersectionPacket intersectPacketTriangle(
        const RayPacket& ray,
        const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2,
        const maskv& active_mask);
};