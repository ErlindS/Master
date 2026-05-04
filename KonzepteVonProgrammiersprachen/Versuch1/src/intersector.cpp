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

Intersection Intersector::intersectRayTriangleBadouel(
    const Ray& ray, 
    const glm::vec3& v0, 
    const glm::vec3& v1, 
    const glm::vec3& v2) 
{
    Intersection result = {false, 0.0f, 0.0f, 0.0f};
    constexpr float EPSILON = 0.0000001f;

    // 1. Ebenengleichung berechnen
    glm::vec3 e1 = v1 - v0;
    glm::vec3 e2 = v2 - v0;
    glm::vec3 n = glm::cross(e1, e2);
    
    // 2. Schnittpunkt mit der Ebene berechnen
    float denom = glm::dot(n, ray.direction);
    if (std::abs(denom) < EPSILON) {
        return result; // Strahl ist parallel zur Ebene
    }
    
    float d = -glm::dot(n, v0);
    float t = -(glm::dot(n, ray.origin) + d) / denom;
    
    if (t < EPSILON) {
        return result; // Schnittpunkt liegt hinter der Kamera
    }
    
    glm::vec3 p = ray.origin + t * ray.direction;
    
    // 3. Dominante Achse der Normalen finden (Projektion auf 2D Ebene)
    float absX = std::abs(n.x);
    float absY = std::abs(n.y);
    float absZ = std::abs(n.z);
    
    int i0, i1;
    if (absX > absY && absX > absZ) {
        i0 = 1; i1 = 2; // Y und Z (projiziere X weg)
    } else if (absY > absZ) {
        i0 = 0; i1 = 2; // X und Z (projiziere Y weg)
    } else {
        i0 = 0; i1 = 1; // X und Y (projiziere Z weg)
    }
    
    // 4. 2D Vektoren in der projizierten Ebene berechnen
    float u0 = p[i0] - v0[i0];
    float v0_2d = p[i1] - v0[i1];
    float u1 = e1[i0];
    float v1_2d = e1[i1];
    float u2 = e2[i0];
    float v2_2d = e2[i1];
    
    // 5. Baryzentrische Koordinaten u und v bestimmen (Gleichungssystem lösen)
    float det = u1 * v2_2d - v1_2d * u2;
    if (std::abs(det) < EPSILON) return result; // Degeneriertes Dreieck
    
    float alpha = (u0 * v2_2d - v0_2d * u2) / det;
    if (alpha < 0.0f || alpha > 1.0f) return result;
    
    float beta = (u1 * v0_2d - v1_2d * u0) / det;
    if (beta < 0.0f || alpha + beta > 1.0f) return result;
    
    // Alles im grünen Bereich -> Treffer!
    result.hit = true;
    result.t = t;
    result.u = alpha;
    result.v = beta;
    return result;
}