#include "renderer.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <random>

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

        // -- Hard Shadows & Blinn-Phong --
        const int N_SHADOW_SAMPLES = 1;
        
        glm::vec3 viewDir = glm::normalize(ray.origin - hitPoint);
        glm::vec3 diffuseSum(0.0f);
        glm::vec3 specularSum(0.0f);

        for (int i = 0; i < N_SHADOW_SAMPLES; ++i) {
            // Exakte Lichtposition (Punktlichtquelle)
            glm::vec3 lightDir = glm::normalize(lightPos - hitPoint);
            float distanceToLight = glm::length(lightPos - hitPoint);
            
            // Schattenstrahl (Shadow Ray)
            Ray shadowRay = {hitPoint + normal * 0.001f, lightDir};
            Intersection shadowIsect;
            Triangle dummyTriangle;
            bool inShadow = findClosestHit(shadowRay, scene.triangles, shadowIsect, dummyTriangle);

            if (inShadow && shadowIsect.t > distanceToLight) {
                inShadow = false;
            }

            if (!inShadow) {
                // Diffuse (Lambert)
                float diff = std::max(glm::dot(normal, lightDir), 0.0f);
                diffuseSum += hitTriangle.color * diff;

                // Specular (Blinn-Phong)
                if (hitTriangle.specularExponent > 0.0f && diff > 0.0f) {
                    glm::vec3 halfDir = glm::normalize(lightDir + viewDir);
                    float specAngle = std::max(glm::dot(normal, halfDir), 0.0f);
                    float specFactor = std::pow(specAngle, hitTriangle.specularExponent);
                    specularSum += hitTriangle.specularColor * specFactor;
                }
            }
        }

        // 15% Umgebungslicht
        glm::vec3 ambientColor = hitTriangle.color * 0.15f;
        
        // Durchschnittliche Beleuchtung über alle Schatten-Samples berechnen
        glm::vec3 finalDiffuse = diffuseSum / (float)N_SHADOW_SAMPLES;
        glm::vec3 finalSpecular = specularSum / (float)N_SHADOW_SAMPLES;

        // Ambient + Diffuse + Specular zusammenrechnen
        glm::vec3 finalColor = glm::clamp(ambientColor + finalDiffuse + finalSpecular, 0.0f, 1.0f);
        return finalColor * 255.0f;
    }
    
    // Hintergrundfarbe (Dunkelblau wie im Referenzbild)
    return glm::vec3(0.05f, 0.05f, 0.15f) * 255.0f;
}

void Renderer::render(const Mesh& scene, const Camera& cam, const glm::vec3& lightPos, Image& image) {
    int width = image.getWidth();
    int height = image.getHeight();
    const int N_AA_SAMPLES = 4;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.5f, 0.5f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            glm::vec3 finalColor(0.0f);
            
            // Anti-Aliasing Loop (Multisampling)
            for (int s = 0; s < N_AA_SAMPLES; ++s) {
                float jitterX = dis(gen);
                float jitterY = dis(gen);
                float u = (float)(x + jitterX) / (width - 1);
                float v = (float)(y + jitterY) / (height - 1);

                Ray ray = cam.generateRay(u, v);
                finalColor += traceRay(ray, scene, lightPos);
            }
            
            // Durchschnittliche Farbe der Samples
            finalColor /= (float)N_AA_SAMPLES;
            
            image.setPixel(x, y, finalColor);
        }
    }
}
