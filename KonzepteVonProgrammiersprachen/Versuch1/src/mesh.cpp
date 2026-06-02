#include "mesh.h"
#include "bvh.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

Mesh::Mesh() : bvh(std::make_unique<BVH>()) {}
Mesh::~Mesh() = default;

bool Mesh::loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Fehler: Konnte " << filename << " nicht oeffnen!" << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;
    
    struct Material {
        glm::vec3 Kd;
        glm::vec3 Ks;
        float Ns;
    };
    
    std::map<std::string, Material> materials;
    Material current_mat = {glm::vec3(0.8f, 0.4f, 0.2f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f}; // Fallback-Material

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "mtllib") {
            std::string mtl_filename;
            iss >> mtl_filename;
            
            // Pfad der OBJ-Datei extrahieren, um die MTL-Datei im selben Ordner zu finden
            std::string basepath = "";
            auto lastSlash = filename.find_last_of('/');
            if (lastSlash != std::string::npos) {
                basepath = filename.substr(0, lastSlash + 1);
            }
            
            std::ifstream mtl_file(basepath + mtl_filename);
            if (mtl_file.is_open()) {
                std::string mtl_line;
                std::string current_mtl = "";
                while (std::getline(mtl_file, mtl_line)) {
                    std::istringstream mtl_iss(mtl_line);
                    std::string mtl_type;
                    mtl_iss >> mtl_type;
                    if (mtl_type == "newmtl") {
                        mtl_iss >> current_mtl;
                    } else if (mtl_type == "Kd" && current_mtl != "") {
                        glm::vec3 kd;
                        mtl_iss >> kd.r >> kd.g >> kd.b;
                        materials[current_mtl].Kd = kd;
                    } else if (mtl_type == "Ks" && current_mtl != "") {
                        glm::vec3 ks;
                        mtl_iss >> ks.r >> ks.g >> ks.b;
                        materials[current_mtl].Ks = ks;
                    } else if (mtl_type == "Ns" && current_mtl != "") {
                        float ns;
                        mtl_iss >> ns;
                        materials[current_mtl].Ns = ns;
                    }
                }
            } else {
                std::cerr << "Warnung: Konnte MTL-Datei " << (basepath + mtl_filename) << " nicht oeffnen!" << std::endl;
            }
        } else if (type == "usemtl") {
            std::string mtl_name;
            iss >> mtl_name;
            if (materials.find(mtl_name) != materials.end()) {
                current_mat = materials[mtl_name];
            }
        } else if (type == "v") {
            // Zeile ist ein Vertex (Eckpunkt)
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        } else if (type == "vn") {
            // Zeile ist eine Vertex-Normale
            glm::vec3 vn;
            iss >> vn.x >> vn.y >> vn.z;
            temp_normals.push_back(vn);
        } else if (type == "f") {
            // Zeile ist ein Face (Dreieck)
            std::string v1_str, v2_str, v3_str;
            iss >> v1_str >> v2_str >> v3_str;

            // Hilfsfunktion zum Parsen von z.B. "1/2/3" oder "1//3" oder "1"
            auto parseFace = [](const std::string& str, int& v_idx, int& n_idx) {
                auto firstSlash = str.find('/');
                if (firstSlash == std::string::npos) {
                    v_idx = std::stoi(str) - 1;
                    n_idx = -1;
                } else {
                    v_idx = std::stoi(str.substr(0, firstSlash)) - 1;
                    auto secondSlash = str.find('/', firstSlash + 1);
                    if (secondSlash != std::string::npos && secondSlash + 1 < str.length()) {
                        n_idx = std::stoi(str.substr(secondSlash + 1)) - 1;
                    } else {
                        n_idx = -1;
                    }
                }
            };

            int v1, v2, v3;
            int n1, n2, n3;
            parseFace(v1_str, v1, n1);
            parseFace(v2_str, v2, n2);
            parseFace(v3_str, v3, n3);

            Triangle tri;
            tri.v0 = temp_vertices[v1];
            tri.v1 = temp_vertices[v2];
            tri.v2 = temp_vertices[v3];
            tri.color = current_mat.Kd;
            tri.specularColor = current_mat.Ks;
            tri.specularExponent = current_mat.Ns;

            // Wenn Normalen vorhanden sind, weisen wir sie zu, andernfalls berechnen wir eine flache Normale (Flat Shading)
            if (n1 >= 0 && n2 >= 0 && n3 >= 0 && 
                n1 < temp_normals.size() && n2 < temp_normals.size() && n3 < temp_normals.size()) {
                tri.n0 = temp_normals[n1];
                tri.n1 = temp_normals[n2];
                tri.n2 = temp_normals[n3];
            } else {
                glm::vec3 flatNormal = glm::normalize(glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
                tri.n0 = flatNormal;
                tri.n1 = flatNormal;
                tri.n2 = flatNormal;
            }

            triangles.push_back(tri);
        }
    }
    
    std::cout << "Geladen: " << triangles.size() << " Dreiecke aus " << filename << std::endl;
    
    std::cout << "Baue Beschleunigungsdatenstruktur (BVH) auf..." << std::endl;
    bvh->build(triangles);
    std::cout << "BVH fertig!" << std::endl;
    
    return true;
}

void Mesh::addGroundPlane(float y, float size, const glm::vec3& color) {
    // Zwei Dreiecke bilden ein großes Quadrat als Bodenebene
    glm::vec3 normal(0.0f, 1.0f, 0.0f); // Normale zeigt nach oben

    Triangle t1;
    t1.v0 = glm::vec3(-size, y, -size);
    t1.v1 = glm::vec3( size, y, -size);
    t1.v2 = glm::vec3( size, y,  size);
    t1.n0 = t1.n1 = t1.n2 = normal;
    t1.color = color;
    t1.specularColor = glm::vec3(0.0f);
    t1.specularExponent = 0.0f;
    triangles.push_back(t1);

    Triangle t2;
    t2.v0 = glm::vec3(-size, y, -size);
    t2.v1 = glm::vec3( size, y,  size);
    t2.v2 = glm::vec3(-size, y,  size);
    t2.n0 = t2.n1 = t2.n2 = normal;
    t2.color = color;
    t2.specularColor = glm::vec3(0.0f);
    t2.specularExponent = 0.0f;
    triangles.push_back(t2);
}