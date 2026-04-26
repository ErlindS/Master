#include <iostream>
#include <glm/glm.hpp>
#include "ray.h"
#include "intersector.h"

int main() {
    // 1. Definiere ein Dreieck in der Welt
    glm::vec3 v0(-1.0f, -1.0f, -5.0f);
    glm::vec3 v1( 1.0f, -1.0f, -5.0f);
    glm::vec3 v2( 0.0f,  1.0f, -5.0f);

    // 2. Erzeuge einen Strahl, der vom Ursprung direkt nach vorne (auf der Z-Achse) schaut
    Ray ray;
    ray.origin = glm::vec3(0.0f, 0.0f, 0.0f);
    ray.direction = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));

    // 3. Teste den Schnittpunkt
    Intersection intersect = Intersector::intersectRayTriangle(ray, v0, v1, v2);

    if (intersect.hit) {
        std::cout << "Treffer! Distanz (t): " << intersect.t << "\n";
        std::cout << "Baryzentrisch (u, v): " << intersect.u << ", " << intersect.v << "\n";
    } else {
        std::cout << "Kein Treffer.\n";
    }

    return 0;
}