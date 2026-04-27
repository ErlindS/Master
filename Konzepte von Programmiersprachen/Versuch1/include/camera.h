#pragma once
#include "ray.h"
#include <glm/glm.hpp>

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 up, float fov, int width, int height) {
        origin = position;
        float aspectRatio = (float)width / (float)height;
        
        // Berechne die Größe des Sichtfeldes (Viewport)
        float theta = glm::radians(fov);
        float h = tan(theta / 2.0f);
        float viewportHeight = 2.0f * h;
        float viewportWidth = aspectRatio * viewportHeight;

        // Berechne das lokale Koordinatensystem der Kamera
        glm::vec3 w = glm::normalize(origin - lookAt);
        glm::vec3 u = glm::normalize(glm::cross(up, w));
        glm::vec3 v = glm::cross(w, u);

        // Vektoren über die Bildebene
        horizontal = viewportWidth * u;
        vertical = viewportHeight * v;
        lowerLeftCorner = origin - horizontal / 2.0f - vertical / 2.0f - w;
    }

    // WICHTIG: Hier nutzen wir jetzt float (s und t bzw. u und v)
    Ray generateRay(float s, float t) const {
        return {origin, glm::normalize(lowerLeftCorner + s * horizontal + t * vertical - origin)};
    }

private:
    glm::vec3 origin;
    glm::vec3 lowerLeftCorner;
    glm::vec3 horizontal;
    glm::vec3 vertical;
};