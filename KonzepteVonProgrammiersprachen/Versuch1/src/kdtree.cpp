#include "bvh.h"
#include <algorithm>
#include <iostream>

void BVH::build(std::vector<Triangle>& triangles) {
    if (triangles.empty()) return;
    
    // Kopiere Dreiecke in unsere interne Liste, um sie zu sortieren
    m_triangles = triangles;
    root = buildRecursive(0, m_triangles.size(), 0);
    
    // Schreibe die sortierten Dreiecke zurück, falls nötig
    triangles = m_triangles;
}

std::unique_ptr<BVHNode> BVH::buildRecursive(int first, int count, int depth) {
    auto node = std::make_unique<BVHNode>();
    
    // 1. Berechne die Bounding Box für alle Dreiecke in diesem Knoten
    for (int i = 0; i < count; ++i) {
        const Triangle& tri = m_triangles[first + i];
        node->bounds.expand(tri.v0);
        node->bounds.expand(tri.v1);
        node->bounds.expand(tri.v2);
    }
    
    // Abbruchbedingung: Wenige Dreiecke oder maximale Tiefe
    if (count <= 2 || depth > 20) {
        node->firstTriangleIndex = first;
        node->triangleCount = count;
        return node;
    }
    
    // 2. Finde die längste Achse der Bounding Box
    glm::vec3 extent = node->bounds.max - node->bounds.min;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;
    
    // 3. Unterteilung am Median
    // Wir sortieren die Dreiecke nach dem Schwerpunkt (Centroid) entlang der gewählten Achse
    std::sort(m_triangles.begin() + first, m_triangles.begin() + first + count,
              [axis](const Triangle& a, const Triangle& b) {
                  float centroidA = (a.v0[axis] + a.v1[axis] + a.v2[axis]) / 3.0f;
                  float centroidB = (b.v0[axis] + b.v1[axis] + b.v2[axis]) / 3.0f;
                  return centroidA < centroidB;
              });
    
    // Der Median-Split erfolgt genau in der Mitte der Liste
    int mid = count / 2;
    
    // Fallback, falls alle Centroids gleich sind (sehr selten, aber sicher ist sicher)
    if (mid == 0 || mid == count) {
        node->firstTriangleIndex = first;
        node->triangleCount = count;
        return node;
    }
    
    // 4. Rekursiver Aufbau der Kinder
    node->left = buildRecursive(first, mid, depth + 1);
    node->right = buildRecursive(first + mid, count - mid, depth + 1);
    
    return node;
}

bool BVH::intersect(const Ray& ray, Intersection& closestIsect, Triangle& hitTriangle, IntersectionAlgorithm algorithm) const {
    closestIsect.hit = false;
    closestIsect.t = std::numeric_limits<float>::max();
    
    if (root) {
        intersectRecursive(root.get(), ray, closestIsect, hitTriangle, algorithm);
    }
    
    return closestIsect.hit;
}

void BVH::intersectRecursive(const BVHNode* node, const Ray& ray, Intersection& closestIsect, Triangle& hitTriangle, IntersectionAlgorithm algorithm) const {
    // Slab-Test für die Bounding Box
    float tMin = 0.0f;
    float tMax = closestIsect.t; // Wir müssen nicht weiter suchen als der bisher nächste Treffer
    if (!node->bounds.intersect(ray, tMin, tMax)) {
        return; // Strahl verfehlt die Bounding Box
    }
    
    if (node->isLeaf()) {
        // Blattknoten: Teste alle Dreiecke in diesem Knoten
        for (int i = 0; i < node->triangleCount; ++i) {
            const Triangle& tri = m_triangles[node->firstTriangleIndex + i];
            Intersection isect;
            
            if (algorithm == IntersectionAlgorithm::BADOUEL) {
                isect = Intersector::intersectRayTriangleBadouel(ray, tri.v0, tri.v1, tri.v2);
            } else {
                isect = Intersector::intersectRayTriangle(ray, tri.v0, tri.v1, tri.v2);
            }
            
            if (isect.hit && isect.t < closestIsect.t) {
                closestIsect = isect;
                hitTriangle = tri;
            }
        }
    } else {
        // Innerer Knoten: Traversiere Kinder
        // TODO (Optimierung): Sortiere die Traversierung nach Distanz (Slab-Test-Ergebnis),
        //                     sodass der nähere Knoten zuerst getestet wird.
        intersectRecursive(node->left.get(), ray, closestIsect, hitTriangle, algorithm);
        intersectRecursive(node->right.get(), ray, closestIsect, hitTriangle, algorithm);
    }
}
