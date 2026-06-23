#include "kdtree.h"
#include <algorithm>
#include <iostream>

void KDTree::build(std::vector<Triangle>& triangles, bool useSAH) {
    if (triangles.empty()) return;
    
    // Kopiere Dreiecke in unsere interne Liste, um sie zu sortieren
    m_triangles = triangles;
    root = buildRecursive(0, m_triangles.size(), 0, useSAH);
    
    // Schreibe die sortierten Dreiecke zurück, falls nötig
    triangles = m_triangles;
}

std::unique_ptr<KDNode> KDTree::buildRecursive(int first, int count, int depth, bool useSAH) {
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
    
    int bestAxis = 0;
    int bestMid = count / 2;
    
    if (!useSAH) {
        // Fallback: Einfacher Median-Split
        glm::vec3 extent = node->bounds.max - node->bounds.min;
        if (extent.y > extent.x) bestAxis = 1;
        if (extent.z > extent[bestAxis]) bestAxis = 2;
        
        std::sort(m_triangles.begin() + first, m_triangles.begin() + first + count,
                  [bestAxis](const Triangle& a, const Triangle& b) {
                      float centroidA = (a.v0[bestAxis] + a.v1[bestAxis] + a.v2[bestAxis]) / 3.0f;
                      float centroidB = (b.v0[bestAxis] + b.v1[bestAxis] + b.v2[bestAxis]) / 3.0f;
                      return centroidA < centroidB;
                  });
    } else {
        // SAH-Split
        float minCost = std::numeric_limits<float>::max();
        float totalArea = node->bounds.surfaceArea();
        
        // Konstanten für SAH
        const float C_trav = 1.0f;
        const float C_isect = 1.5f; // Dreiecksschnitt ist etwas teurer als AABB-Schnitt
        
        float leafCost = count * C_isect; // Kosten, wenn dieser Knoten ein Blatt wird
        
        // Wir probieren alle 3 Achsen
        for (int axis = 0; axis < 3; ++axis) {
            // Sortieren nach Centroid entlang der aktuellen Achse
            std::sort(m_triangles.begin() + first, m_triangles.begin() + first + count,
                      [axis](const Triangle& a, const Triangle& b) {
                          float centroidA = (a.v0[axis] + a.v1[axis] + a.v2[axis]) / 3.0f;
                          float centroidB = (b.v0[axis] + b.v1[axis] + b.v2[axis]) / 3.0f;
                          return centroidA < centroidB;
                      });
            
            // Sweep von Links nach Rechts
            std::vector<float> leftArea(count);
            AABB leftBox;
            for (int i = 0; i < count; ++i) {
                const Triangle& tri = m_triangles[first + i];
                leftBox.expand(tri.v0);
                leftBox.expand(tri.v1);
                leftBox.expand(tri.v2);
                leftArea[i] = leftBox.surfaceArea();
            }
            
            // Sweep von Rechts nach Links
            std::vector<float> rightArea(count);
            AABB rightBox;
            for (int i = count - 1; i >= 0; --i) {
                const Triangle& tri = m_triangles[first + i];
                rightBox.expand(tri.v0);
                rightBox.expand(tri.v1);
                rightBox.expand(tri.v2);
                rightArea[i] = rightBox.surfaceArea();
            }
            
            // Finde den besten Split auf dieser Achse
            for (int i = 1; i < count; ++i) {
                // i Elemente links (0 bis i-1), count - i Elemente rechts (i bis count-1)
                float probLeft = leftArea[i - 1] / totalArea;
                float probRight = rightArea[i] / totalArea;
                
                float cost = C_trav + C_isect * (probLeft * i + probRight * (count - i));
                
                if (cost < minCost) {
                    minCost = cost;
                    bestAxis = axis;
                    bestMid = i;
                }
            }
        }
        
        // Wenn selbst der beste Split schlechter ist als ein Blattknoten, mache ein Blatt daraus
        // Optional: Kleine Toleranz einbauen oder strikt nach SAH vorgehen.
        if (minCost > leafCost) {
            node->firstTriangleIndex = first;
            node->triangleCount = count;
            return node;
        }
        
        // Wir müssen die Dreiecke endgültig nach der besten Achse sortieren
        std::sort(m_triangles.begin() + first, m_triangles.begin() + first + count,
                  [bestAxis](const Triangle& a, const Triangle& b) {
                      float centroidA = (a.v0[bestAxis] + a.v1[bestAxis] + a.v2[bestAxis]) / 3.0f;
                      float centroidB = (b.v0[bestAxis] + b.v1[bestAxis] + b.v2[bestAxis]) / 3.0f;
                      return centroidA < centroidB;
                  });
    }
    
    // Fallback, falls alle Centroids gleich sind (sehr selten, aber sicher ist sicher)
    if (bestMid == 0 || bestMid == count) {
        node->firstTriangleIndex = first;
        node->triangleCount = count;
        return node;
    }
    
    // 4. Rekursiver Aufbau der Kinder
    node->left = buildRecursive(first, bestMid, depth + 1, useSAH);
    node->right = buildRecursive(first + bestMid, count - bestMid, depth + 1, useSAH);
    
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

// Einstiegspunkt für die Traversierung eines Strahlenpakets durch den KD-Baum
void KDTree::intersectPacket(const RayPacket& ray, const maskv& active_mask, IntersectionPacket& closestIsect, IntersectionAlgorithm algorithm) const {
    if (root) {
        intersectPacketRecursive(root.get(), ray, active_mask, closestIsect, algorithm);
    }
}

// Rekursive Traversierung des KD-Baums für ein ganzes Paket an Strahlen
void KDTree::intersectPacketRecursive(const KDNode* node, const RayPacket& ray, const maskv& active_mask, IntersectionPacket& closestIsect, IntersectionAlgorithm algorithm) const {
    // Vektorisierter Slab-Test für die Bounding Box des aktuellen Knotens
    floatv tMin = floatv(0.0f);
    floatv tMax = closestIsect.t; // Strahlen können abbrechen, wenn sie schon einen näheren Treffer haben
    maskv hitMask = node->bounds.intersectPacket(ray, tMin, tMax) && active_mask;
    
    // Abbruch, wenn kein einziger Strahl im Paket die Bounding Box trifft
    if (stdx::none_of(hitMask)) {
        return;
    }
    
    if (node->isLeaf()) {
        // Blattknoten: Teste alle Dreiecke in diesem Knoten vektorisiert
        for (int i = 0; i < node->triangleCount; ++i) {
            int triIdx = node->firstTriangleIndex + i;
            const Triangle& tri = m_triangles[triIdx];
            
            IntersectionPacket isect = Intersector::intersectPacketTriangle(ray, tri.v0, tri.v1, tri.v2, hitMask);
            
            // Maske für alle Strahlen, die das Dreieck getroffen haben UND näher sind als vorherige Treffer
            maskv closerHit = isect.hit && (isect.t < closestIsect.t);
            if (stdx::any_of(closerHit)) {
                // Bedingte Zuweisung (where) nur für die Strahlen, bei denen die Maske zutrifft
                stdx::where(closerHit, closestIsect.hit) = true;
                stdx::where(closerHit, closestIsect.t) = isect.t;
                stdx::where(closerHit, closestIsect.u) = isect.u;
                stdx::where(closerHit, closestIsect.v) = isect.v;
                stdx::where(closerHit, closestIsect.triIndex) = floatv((float)triIdx);
            }
        }
    } else {
        // Innerer Knoten: Teste zunächst das linke Kind mit der aktuellen hitMask
        intersectPacketRecursive(node->left.get(), ray, hitMask, closestIsect, algorithm);
        
        // Vor dem Test des rechten Kindes evaluieren wir den Slab-Test neu.
        // Das ist wichtig, da sich closestIsect.t durch Treffer im linken Kind verkleinert haben könnte!
        floatv tMinR = floatv(0.0f);
        floatv tMaxR = closestIsect.t;
        maskv hitMaskR = node->right->bounds.intersectPacket(ray, tMinR, tMaxR) && hitMask;
        
        // Nur in das rechte Kind absteigen, wenn immer noch mindestens ein Strahl die Bounding Box trifft
        if (stdx::any_of(hitMaskR)) {
            intersectPacketRecursive(node->right.get(), ray, hitMaskR, closestIsect, algorithm);
        }
    }
}
