#ifndef PPM_READER_H
#define PPM_READER_H

#include <vector>
#include <tuple>
#include <sstream>
#include <iostream>

struct Image {
    unsigned int width;
    unsigned int height;
    unsigned int max_color_value;
    std::vector<unsigned int> pixels;  // Store pixels as a flat array of unsigned ints, R, G, B for each pixel

    // Default constructor
    Image() : width(0), height(0), max_color_value(0) {}

    // Parameterized constructor
    Image(unsigned int w, unsigned int h, unsigned int max) 
        : width(w), height(h), max_color_value(max) {
        pixels.resize(w * h * 3, 0);  // 3 for RGB
    }

    // Method to return a pointer to the pixel data
    unsigned int* get_pixels_data() {
        if (!pixels.empty()) {
            return &pixels[0];
        }
        return nullptr;
    }
	
// Method to get a pixel at (x, y)
    std::tuple<unsigned int, unsigned int, unsigned int> get_pixel(unsigned int x, unsigned int y) const {
        if (x >= width || y >= height) {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }
        unsigned int index = (y * width + x) * 3;
        return std::make_tuple(pixels[index], pixels[index + 1], pixels[index + 2]);
    }
	
	// Method to write the image to an output stream in PPM P3 format
    void write_ppm(std::ostream& out) const;
};

Image read_ppm(std::istringstream& ppm_data);

#endif // PPM_READER_H