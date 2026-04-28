#include "intersector.h"

Intersection Intersector::intersectRayTriangle(
    const Ray& ray, 
    const glm::vec3& v0, 
    const glm::vec3& v1, 
    const glm::vec3& v2) 
{
    Intersection result = {false, 0.0f, 0.0f, 0.0f};

    // Kanten des Dreiecks berechnen
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    // Determinante berechnen (auch Vektor p genannt)
    // Einfach der orthogonale Vektor
    glm::vec3 h = glm::cross(ray.direction, edge2);

    
    float a = glm::dot(edge1, h);

    // Wenn a nahe 0 ist, ist der Strahl parallel zum Dreieck
    constexpr float EPSILON = 0.0000001f;
    if (a > -EPSILON && a < EPSILON) {
        return result; // Kein Schnittpunkt
    }

    float f = 1.0f / a;
    glm::vec3 s = ray.origin - v0;
    
    // u-Parameter berechnen und prüfen
    result.u = f * glm::dot(s, h);
    if (result.u < 0.0f || result.u > 1.0f) {
        return result;
    }

    glm::vec3 q = glm::cross(s, edge1);
    
    // v-Parameter berechnen und prüfen
    result.v = f * glm::dot(ray.direction, q);
    if (result.v < 0.0f || result.u + result.v > 1.0f) {
        return result;
    }

    // Ab hier wissen wir: Der Strahl trifft das Dreieck!
    // Jetzt t berechnen, um herauszufinden, wo der Schnittpunkt liegt
    result.t = f * glm::dot(edge2, q);

    // Nur Treffer in Blickrichtung zählen (t > EPSILON)
    if (result.t > EPSILON) {
        result.hit = true;
        return result;
    }

    return result; // Treffer lag hinter der Kamera
}