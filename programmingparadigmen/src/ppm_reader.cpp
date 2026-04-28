#include "ppm_reader.h"
#include <sstream>
#include <string>
#include <iostream>

// Implementation of write_ppm method
void Image::write_ppm(std::ostream& out) const {
    if (width == 0 || height == 0) {
        throw std::runtime_error("Image dimensions cannot be zero.");
    }

    out << "P3\n"; // Magic number for PPM ASCII
    out << width << " " << height << "\n";
    out << max_color_value << "\n";

    // Write pixel data
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            unsigned int index = (y * width + x) * 3;
            out << pixels[index] << " " << pixels[index + 1] << " " << pixels[index + 2];
            if (x < width - 1) {
                out << " "; // Space between pixels in a row
            }
        }
        out << "\n"; // New line for each row
    }
}
	
// Helper function to strip comments from a line
std::string strip_comments(const std::string& line) {
    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos) {
        return line.substr(0, comment_pos);
    }
    return line;
}

// Function to read PPM ASCII format from a string input stream, ignoring comments at any position
Image read_ppm(std::istringstream& ppm_data) {
    std::string magic_number;
    unsigned int width, height, max_color_value;
    
    // Read magic number, stripping comments
    std::string line;
    while (std::getline(ppm_data, line)) {
        std::istringstream iss(strip_comments(line));
        if (iss >> magic_number) {
            if (magic_number != "P3") {
                throw std::runtime_error("Invalid file format. Expected P3 format.");
            }
            break; // Found the magic number, stop looking
        }
    }

    // Read width, height, and max color value, stripping comments
    while (std::getline(ppm_data, line)) {
        std::istringstream iss(strip_comments(line));
        if (iss >> width >> height) {
            std::getline(ppm_data, line);
            iss = std::istringstream(strip_comments(line));
            if (iss >> max_color_value) {
                break; // Read all necessary header information
            }
        }
    }

    Image img(width, height, max_color_value);

    // Read pixel data, stripping comments and handling any number of pixels per line
    unsigned int pixel_count = 0;
    while (pixel_count < width * height * 3) {  // 3 for RGB
        if (!std::getline(ppm_data, line)) {
            throw std::runtime_error("Unexpected end of stream while reading pixel data.");
        }
        
        std::istringstream pixel_line(strip_comments(line));
        unsigned int value;
        while (pixel_line >> value && pixel_count < width * height * 3) {
            img.pixels[pixel_count++] = value;
        }
    }

    return img;
}