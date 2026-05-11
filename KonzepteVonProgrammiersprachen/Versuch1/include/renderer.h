#pragma once

#include "mesh.h"
#include "camera.h"
#include "image.h"
#include "intersector.h"
#include <glm/glm.hpp>
#include <vector>


class Renderer {
public:
    void setAlgorithm(IntersectionAlgorithm alg) { algorithm = alg; }

    // Durchläuft alle Pixel und rendert die Szene
    void render(const Mesh& scene, const Camera& cam, const glm::vec3& lightPos, Image& image);

private:
    IntersectionAlgorithm algorithm = IntersectionAlgorithm::MOELLER_TRUMBORE;

    // Sucht das nächste getroffene Dreieck in der Szene mittels BVH
    bool findClosestHit(const Ray& ray, const Mesh& scene, Intersection& closestIsect, Triangle& hitTriangle);

    // Berechnet die Farbe für einen einzelnen Strahl
    glm::vec3 traceRay(const Ray& ray, const Mesh& scene, const glm::vec3& lightPos);
};
