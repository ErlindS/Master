#include "image.h"
#include <fstream>
#include <iostream>
#include <algorithm>

Image::Image(int w, int h) : width(w), height(h) {
    pixels.resize(width * height, glm::vec3(0.0f));
}

void Image::setPixel(int x, int y, const glm::vec3& color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        // PPM expects pixels from top to bottom, but often raytracers render bottom to top.
        // We'll store it directly, but typically y is top-down in the loop now, wait.
        // In our main.cpp, the loop was: for (int y = height - 1; y >= 0; --y)
        // If we want index 0 to be the first written pixel (top-left), we'll do:
        // Actually, we can just let Renderer pass the correct array index or we manage it here.
        // To keep it simple, we just map (x, y) where (0,0) is bottom-left (standard math coordinates).
        // Then we save it top-down.
        pixels[y * width + x] = color;
    }
}

bool Image::save(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Fehler beim Erstellen von " << filename << std::endl;
        return false;
    }

    file << "P3\n" << width << " " << height << "\n255\n";
    
    // We iterate from top (height - 1) to bottom (0)
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            const glm::vec3& color = pixels[y * width + x];
            // Clamp colors to [0, 255] just in case
            int r = (int)(std::clamp(color.r, 0.0f, 255.0f));
            int g = (int)(std::clamp(color.g, 0.0f, 255.0f));
            int b = (int)(std::clamp(color.b, 0.0f, 255.0f));
            file << r << " " << g << " " << b << "\n";
        }
    }

    file.close();
    return true;
}
