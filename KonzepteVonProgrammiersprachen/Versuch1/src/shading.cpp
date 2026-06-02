#include "shading.h"
#include <algorithm>
#include <limits>

bool Shading::findClosestHit(
    const Ray& ray,
    const std::vector<Triangle>& triangles,
    Intersection& closestIsect,
    Triangle& hitTriangle,
    IntersectionAlgorithm algorithm)
{
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

bool Shading::isInShadow(
    const glm::vec3& hitPoint,
    const glm::vec3& normal,
    const glm::vec3& lightPos,
    const std::vector<Triangle>& triangles,
    IntersectionAlgorithm algorithm)
{
    glm::vec3 lightDir = glm::normalize(lightPos - hitPoint);
    float distanceToLight = glm::length(lightPos - hitPoint);

    // Schattenstrahl (Shadow Ray) – leicht versetzt, um Shadow Acne zu vermeiden
    Ray shadowRay = {hitPoint + normal * 0.001f, lightDir};
    Intersection shadowIsect;
    Triangle dummyTriangle;
    bool inShadow = findClosestHit(shadowRay, triangles, shadowIsect, dummyTriangle, algorithm);

    // Nur Objekte ZWISCHEN Trefferpunkt und Lichtquelle erzeugen Schatten
    if (inShadow && shadowIsect.t > distanceToLight) {
        inShadow = false;
    }

    return inShadow;
}

glm::vec3 Shading::computeShading(
    const glm::vec3& hitPoint,
    const glm::vec3& normal,
    const glm::vec3& viewDir,
    const Triangle& hitTriangle,
    const Light& light,
    const std::vector<Triangle>& triangles,
    IntersectionAlgorithm algorithm)
{
    // Lichtrichtung berechnen
    glm::vec3 lightDir = glm::normalize(light.position - hitPoint);

    // Schatten prüfen
    bool inShadow = isInShadow(hitPoint, normal, light.position, triangles, algorithm);

    glm::vec3 diffuseColor(0.0f);
    glm::vec3 specularColor(0.0f);

    if (!inShadow) {
        // Diffuse (Lambert)
        float diff = std::max(glm::dot(normal, lightDir), 0.0f);
        diffuseColor = hitTriangle.color * diff;

        // Specular (Blinn-Phong)
        if (hitTriangle.specularExponent > 0.0f && diff > 0.0f) {
            glm::vec3 halfDir = glm::normalize(lightDir + viewDir);
            float specAngle = std::max(glm::dot(normal, halfDir), 0.0f);
            float specFactor = std::pow(specAngle, hitTriangle.specularExponent);
            specularColor = hitTriangle.specularColor * specFactor;
        }
    }

    // 15% Umgebungslicht
    glm::vec3 ambientColor = hitTriangle.color * 0.15f;

    // Ambient + Diffuse + Specular zusammenrechnen
    glm::vec3 finalColor = glm::clamp(ambientColor + diffuseColor + specularColor, 0.0f, 1.0f);
    return finalColor * 255.0f;
}
