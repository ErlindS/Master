#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

class Image {
public:
    Image(int width, int height);

    void setPixel(int x, int y, const glm::vec3& color);
    bool save(const std::string& filename) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width;
    int height;
    std::vector<glm::vec3> pixels;
};
