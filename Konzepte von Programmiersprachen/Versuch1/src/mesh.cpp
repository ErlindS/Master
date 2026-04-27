#include "mesh.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool Mesh::loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Fehler: Konnte " << filename << " nicht oeffnen!" << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_vertices;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            // Zeile ist ein Vertex (Eckpunkt)
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        } else if (type == "f") {
            // Zeile ist ein Face (Dreieck)
            // Format ist oft: v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            std::string v1_str, v2_str, v3_str;
            iss >> v1_str >> v2_str >> v3_str;

            // Wir schneiden alles ab dem ersten Slash '/' ab, um nur den Vertex-Index zu bekommen
            int i1 = std::stoi(v1_str.substr(0, v1_str.find('/'))) - 1;
            int i2 = std::stoi(v2_str.substr(0, v2_str.find('/'))) - 1;
            int i3 = std::stoi(v3_str.substr(0, v3_str.find('/'))) - 1;

            triangles.push_back({temp_vertices[i1], temp_vertices[i2], temp_vertices[i3]});
        }
    }
    
    std::cout << "Geladen: " << triangles.size() << " Dreiecke aus " << filename << std::endl;
    return true;
}