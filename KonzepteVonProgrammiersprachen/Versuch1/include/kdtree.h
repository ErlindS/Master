#pragma once
#include <vector>
#include <memory>
#include <limits>
#include <glm/glm.hpp>
#include "mesh.h"
#include "ray.h"
#include "intersector.h"
#include "packet.h"

// Axis-Aligned Bounding Box für den Slab-Test
struct AABB {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

    AABB() = default;
    
    // Erweitert die Bounding Box um einen Punkt
    void expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }
    
    // Erweitert die Bounding Box um eine andere Box
    void expand(const AABB& other) {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }
    
    // Gibt den Mittelpunkt der Box zurück (wichtig für den Median-Split)
    glm::vec3 centroid() const {
        return (min + max) * 0.5f;
    }

    // Berechnet die Oberfläche der Bounding Box (für SAH)
    float surfaceArea() const {
        glm::vec3 extent = max - min;
        if (extent.x <= 0.0f || extent.y <= 0.0f || extent.z <= 0.0f) return 0.0f;
        return 2.0f * (extent.x * extent.y + extent.x * extent.z + extent.y * extent.z);
    }

    // Slab-Test: Prüft, ob der Strahl die Box trifft
    // ray.invDirection ist 1.0f / ray.direction (zur Vermeidung von Divisionen)
    bool intersect(const Ray& ray, float& tMin, float& tMax) const {
        glm::vec3 t0 = (min - ray.origin) * ray.invDirection;
        glm::vec3 t1 = (max - ray.origin) * ray.invDirection;

        glm::vec3 tSmall = glm::min(t0, t1);
        glm::vec3 tBig   = glm::max(t0, t1);

        tMin = std::max(tMin, std::max(tSmall.x, std::max(tSmall.y, tSmall.z)));
        tMax = std::min(tMax, std::min(tBig.x, std::min(tBig.y, tBig.z)));

        return tMin <= tMax;
    }
    
    // Slab-Test für SIMD Packet
    maskv intersectPacket(const RayPacket& ray, floatv& tMin, floatv& tMax) const {
        floatv t0x = (floatv(min.x) - ray.ox) * ray.invDx;
        floatv t1x = (floatv(max.x) - ray.ox) * ray.invDx;
        
        floatv t0y = (floatv(min.y) - ray.oy) * ray.invDy;
        floatv t1y = (floatv(max.y) - ray.oy) * ray.invDy;
        
        floatv t0z = (floatv(min.z) - ray.oz) * ray.invDz;
        floatv t1z = (floatv(max.z) - ray.oz) * ray.invDz;

        floatv tSmallX = stdx::min(t0x, t1x);
        floatv tBigX   = stdx::max(t0x, t1x);
        
        floatv tSmallY = stdx::min(t0y, t1y);
        floatv tBigY   = stdx::max(t0y, t1y);
        
        floatv tSmallZ = stdx::min(t0z, t1z);
        floatv tBigZ   = stdx::max(t0z, t1z);

        tMin = stdx::max(tMin, stdx::max(tSmallX, stdx::max(tSmallY, tSmallZ)));
        tMax = stdx::min(tMax, stdx::min(tBigX, stdx::min(tBigY, tBigZ)));

        return tMin <= tMax;
    }
};

// Ein Knoten im Baum (kann innerer Knoten oder Blatt sein)
struct KDNode {
    AABB bounds;
    std::unique_ptr<KDNode> left;
    std::unique_ptr<KDNode> right;
    
    // Für Blatt-Knoten: Referenz auf die Dreiecke im Array
    int firstTriangleIndex = -1;
    int triangleCount = 0;
    
    bool isLeaf() const { return triangleCount > 0; }
};

// Die Hauptklasse, die den Baum verwaltet
class KDTree {
public:
    // Baut den Baum mittels SAH oder Median-Split
    void build(std::vector<Triangle>& triangles, bool useSAH = true);
    
    // Traversiert den Baum und sucht den exakten, nächsten Schnittpunkt
    bool intersect(const Ray& ray, Intersection& closestIsect, Triangle& hitTriangle, IntersectionAlgorithm algorithm) const;
    
    // Traversiert den Baum mit einem Strahlenpaket
    void intersectPacket(const RayPacket& ray, const maskv& active_mask, IntersectionPacket& closestIsect, IntersectionAlgorithm algorithm) const;

private:
    std::unique_ptr<KDNode> root;
    std::vector<Triangle> m_triangles; // Interne, umsortierte Liste der Dreiecke
    
    // Rekursive Hilfsfunktion für den Aufbau
    std::unique_ptr<KDNode> buildRecursive(int first, int count, int depth, bool useSAH);
    
    // Rekursive Traversierungsfunktion
    void intersectRecursive(const KDNode* node, const Ray& ray, Intersection& closestIsect, Triangle& hitTriangle, IntersectionAlgorithm algorithm) const;
    
    // Rekursive Traversierungsfunktion für Pakete
    void intersectPacketRecursive(const KDNode* node, const RayPacket& ray, const maskv& active_mask, IntersectionPacket& closestIsect, IntersectionAlgorithm algorithm) const;
};
