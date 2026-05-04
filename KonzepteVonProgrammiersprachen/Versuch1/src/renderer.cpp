#include "renderer.h"
#include <iostream>
#include <limits>
#include <algorithm>

bool Renderer::findClosestHit(const Ray& ray, const std::vector<Triangle>& triangles, Intersection& closestIsect, Triangle& hitTriangle) {
    closestIsect.hit = false;
    closestIsect.t = std::numeric_limits<float>::max();
    bool hitAnything = false;

    for (const auto& tri : triangles) {
        Intersection isect;
        if (algorithm == IntersectionAlgorithm::BADOUEL) {
            isect = Intersector::intersectRayTriangleBadouel(ray, tri.v0, tri.v1, tri.v2);
        } else {
            isect = Intersector::intersectRayTriangle(ray, tri.v0, tri.v1, tri.v2);
        }
        
        if (isect.hit && isect.t < closestIsect.t) {
            closestIsect = isect;
            hitTriangle = tri;
            hitAnything = true;
        }
    }
    return hitAnything;
}

glm::vec3 Renderer::traceRay(const Ray& ray, const Mesh& scene, const glm::vec3& lightPos) {
    Intersection isect;
    Triangle hitTriangle;

    if (findClosestHit(ray, scene.triangles, isect, hitTriangle)) {
        // Exakter Punkt im 3D-Raum (Ursprung + Richtung * Distanz)
        glm::vec3 hitPoint = ray.origin + ray.direction * isect.t;

        // Oberflächennormale berechnen (Interpoliert mit baryzentrischen Koordinaten)
        float w = 1.0f - isect.u - isect.v;
        glm::vec3 normal = glm::normalize(w * hitTriangle.n0 + isect.u * hitTriangle.n1 + isect.v * hitTriangle.n2);

        // Vektor vom Trefferpunkt zum Licht
        glm::vec3 lightDir = glm::normalize(lightPos - hitPoint);

        // Schattenstrahl (Shadow Ray)
        float distanceToLight = glm::length(lightPos - hitPoint);
        Ray shadowRay = {hitPoint + normal * 0.001f, lightDir};
        Intersection shadowIsect;
        Triangle dummyTriangle;
        bool inShadow = findClosestHit(shadowRay, scene.triangles, shadowIsect, dummyTriangle);

        if (inShadow && shadowIsect.t > distanceToLight) {
            inShadow = false;
        }

        // Objektfarbe aus dem getroffenen Dreieck holen (Materialfarbe aus der .mtl Datei)
        glm::vec3 objectColor = hitTriangle.color;
        glm::vec3 ambientColor = objectColor * 0.15f; // 15% Umgebungslicht

        if (inShadow) {
            // Nur Umgebungslicht
            return ambientColor * 255.0f;
        } else {
            // Basisfarbe berechnen (Lambert Diffuse)
            float diff = std::max(glm::dot(normal, lightDir), 0.0f);
            glm::vec3 diffuseColor = objectColor * diff;
            
            // Ambient + Diffuse zusammenrechnen
            glm::vec3 finalColor = glm::clamp(ambientColor + diffuseColor, 0.0f, 1.0f);
            return finalColor * 255.0f;
        }
    }
    
    // Hintergrundfarbe (Schwarz)
    return glm::vec3(0.0f, 0.0f, 0.0f);
}

void Renderer::render(const Mesh& scene, const Camera& cam, const glm::vec3& lightPos, Image& image) {
    int width = image.getWidth();
    int height = image.getHeight();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = (float)x / (width - 1);
            float v = (float)y / (height - 1);

            Ray ray = cam.generateRay(u, v);
            glm::vec3 color = traceRay(ray, scene, lightPos);
            
            image.setPixel(x, y, color);
        }
    }
}
