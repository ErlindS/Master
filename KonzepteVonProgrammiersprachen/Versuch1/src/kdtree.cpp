#include "kdtree.h"
#include <algorithm>
#include <iostream>

void KDTree::build(std::vector<Triangle>& triangles) {
    if (triangles.empty()) return;
    
    // Kopiere Dreiecke in unsere interne Liste, um sie zu sortieren
    m_triangles = triangles;
    root = buildRecursive(0, m_triangles.size(), 0);
    
    // Schreibe die sortierten Dreiecke zurück, falls nötig
    triangles = m_triangles;
}

std::unique_ptr<KDNode> KDTree::buildRecursive(int first, int count, int depth) {
    auto node = std::make_unique<KDNode>();
    
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

bool KDTree::intersect(const Ray& ray, Intersection& closestIsect, Triangle& hitTriangle, IntersectionAlgorithm algorithm) const {
    closestIsect.hit = false;
    closestIsect.t = std::numeric_limits<float>::max();
    
    if (root) {
        intersectRecursive(root.get(), ray, closestIsect, hitTriangle, algorithm);
    }
    
    return closestIsect.hit;
}

void KDTree::intersectRecursive(const KDNode* node, const Ray& ray, Intersection& closestIsect, Triangle& hitTriangle, IntersectionAlgorithm algorithm) const {
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
        // Innerer Knoten: Front-to-Back Traversierung
        // Führe den Slab-Test für beide Kinder durch, um die Distanz (tMin) zu erhalten
        float tMinL = 0.0f, tMaxL = closestIsect.t;
        bool hitL = node->left->bounds.intersect(ray, tMinL, tMaxL);
        
        float tMinR = 0.0f, tMaxR = closestIsect.t;
        bool hitR = node->right->bounds.intersect(ray, tMinR, tMaxR);

        if (hitL && hitR) {
            // Beide Kinder getroffen: Teste den näheren Knoten ZUERST!
            if (tMinL < tMinR) {
                intersectRecursive(node->left.get(), ray, closestIsect, hitTriangle, algorithm);
                // WICHTIG: Nach dem linken Ast prüfen wir, ob wir den rechten Ast noch brauchen!
                // Wenn wir im linken Ast einen Treffer fanden, ist closestIsect.t jetzt kleiner.
                // Liegt der rechte Ast weiter weg als unser neuer Treffer, können wir ihn ignorieren.
                if (tMinR < closestIsect.t) { 
                    intersectRecursive(node->right.get(), ray, closestIsect, hitTriangle, algorithm);
                }
            } else {
                intersectRecursive(node->right.get(), ray, closestIsect, hitTriangle, algorithm);
                if (tMinL < closestIsect.t) {
                    intersectRecursive(node->left.get(), ray, closestIsect, hitTriangle, algorithm);
                }
            }
        } else if (hitL) {
            intersectRecursive(node->left.get(), ray, closestIsect, hitTriangle, algorithm);
        } else if (hitR) {
            intersectRecursive(node->right.get(), ray, closestIsect, hitTriangle, algorithm);
        }
    }
}
