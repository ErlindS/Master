#include "obj_loader.h"
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <sstream>
#include <optional>

namespace wavefront {


void ObjLoader::setDefaultMaterial(Material material) {
    if (materials.find(defaultMaterialName) != materials.end()) {
        materials.erase(defaultMaterialName);
    }

    material.name = defaultMaterialName;
    materials.insert(std::make_pair(defaultMaterialName, std::move(material)));

    auto materialGroupIterator = materialGroups.find(defaultMaterialName);
    if (materialGroupIterator != materialGroups.end()) {
        materialGroups.erase(defaultMaterialName);
    }
    auto [it, inserted] = materialGroups.try_emplace(defaultMaterialName, defaultMaterialName);
    currentMaterial = it->first;
}

Material& ObjLoader::getDefaultMaterial() {
    if (materials.find(defaultMaterialName) == materials.end()) {
        Material defaultMaterial;
        defaultMaterial.name = defaultMaterialName;

        // Set default values for members without default initializers
        defaultMaterial.ambient = std::make_unique<Vector3f>(Vector3f{0.2f, 0.2f, 0.2f});
        defaultMaterial.diffuse = std::make_unique<Vector3f>(Vector3f{0.8f, 0.8f, 0.8f});
        defaultMaterial.specular = std::make_unique<Vector3f>(Vector3f{0.0f, 0.0f, 0.0f});
        defaultMaterial.shininess = std::make_unique<float>(10.0f);
        defaultMaterial.transmission = std::make_unique<float>(0.0f);
        defaultMaterial.opticalDensity = std::make_unique<float>(1.0f);
        defaultMaterial.diffuseTexture = nullptr;
        defaultMaterial.ambientTexture = nullptr;
        defaultMaterial.specularTexture = nullptr;
        defaultMaterial.specularHighlightTexture = nullptr;
        defaultMaterial.alphaTexture = nullptr;
        defaultMaterial.bumpMap = nullptr;

        materials.insert(std::make_pair(defaultMaterialName, std::move(defaultMaterial)));
		auto [it, inserted] = materialGroups.try_emplace(defaultMaterialName, defaultMaterialName);
		currentMaterial = it->first;
    }

    return materials[defaultMaterialName];
}

// Helper function to strip comments from a line
std::string ObjLoader::strip_comments(const std::string& line) {
    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos) {
        return line.substr(0, comment_pos);
    }
    return line;
}

// Helper function to split a string by delimiter
std::vector<std::string> ObjLoader::split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


wavefront::Face ObjLoader::parseFace(const std::string& faceString) {
    Face face;
    std::vector<std::string> vertexDefinitions = split(faceString, ' '); // Split into vertex definitions

    if (vertexDefinitions.empty()) {
        throw std::runtime_error("Empty face definition");
    }

    for (const auto& vertexDef : vertexDefinitions) {
        std::vector<std::string> indices = split(vertexDef, '/');

        int vertexIndex = -1;
        int textureIndex = -1;
        int normalIndex = -1;

        try {
            if (!indices.empty() && !indices[0].empty()) {
                vertexIndex = std::stoi(indices[0]) - 1;  // OBJ uses 1-based indexing
                if (vertexIndex < 0 || vertexIndex >= vertices.size()) {
                    throw std::runtime_error("Vertex index out of range");
                }
            }

            if (indices.size() > 1 && !indices[1].empty()) {
                textureIndex = std::stoi(indices[1]) - 1;  // OBJ uses 1-based indexing
                if (textureIndex < 0 || textureIndex >= texcoords.size()) {
                    throw std::runtime_error("Texture index out of range");
                }
            } else {
				textureIndex = -1;
			}

            if (indices.size() > 2 && !indices[2].empty()) {
                normalIndex = std::stoi(indices[2]) - 1;  // OBJ uses 1-based indexing
                if (normalIndex < 0 || normalIndex >= normals.size()) {
                    throw std::runtime_error("Normal index out of range");
                }
            } else {
				normalIndex = -1;
			}

        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid integer conversion for face indices: " + std::string(e.what()));
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Face index out of range: " + std::string(e.what()));
        }

        face.vertexIndices.push_back(vertexIndex);
        face.textureIndices.push_back(textureIndex);
        face.normalIndices.push_back(normalIndex);
    }

    return face;
}
// Parse texture options from the string part following the texture filename in MTL
void ObjLoader::parseTextureOptions(TextureMap& texture, const std::string& options) {
    std::istringstream iss(strip_comments(options));
    std::string option;
    while (iss >> option) {
        if (option == "-clamp") {
            texture.clamp = std::make_unique<bool>(true);
        } else if (option == "-bm") {
            float bumpMultiplier;
            if (iss >> bumpMultiplier) {
                texture.bumpMultiplier = std::make_unique<float>(bumpMultiplier);
            } else {
                throw std::runtime_error("Failed to parse bump multiplier for texture: " + options);
            }
        } else if (option == "-blendu" || option == "-blendv") {
            std::string blendValue;
            if (iss >> blendValue) {
                bool blend = (blendValue != "off"); // Assuming "off" means false, anything else means true
                if (option == "-blendu") {
                    texture.blendU = std::make_unique<bool>(blend);
                    texture.blendMode = std::make_unique<BlendMode>(blend ? BlendMode::BlendU : BlendMode::Modulate);
                } else {
                    texture.blendV = std::make_unique<bool>(blend);
                    texture.blendMode = std::make_unique<BlendMode>(blend ? BlendMode::BlendV : BlendMode::Modulate);
                }
            } else {
                throw std::runtime_error("Failed to parse " + option + " for texture: " + options);
            }
        } else if (option == "-o") {
            std::string offset;
            if (iss >> offset) {
                texture.offset = std::make_unique<std::string>(offset);
            } else {
                throw std::runtime_error("Failed to parse offset for texture: " + options);
            }
        } else if (option == "-s") {
            std::string scale;
            if (iss >> scale) {
                texture.scale = std::make_unique<std::string>(scale);
            } else {
                throw std::runtime_error("Failed to parse scale for texture: " + options);
            }
        } else if (option == "-t") {
            std::string turbulence;
            if (iss >> turbulence) {
                texture.turbulence = std::make_unique<std::string>(turbulence);
            } else {
                throw std::runtime_error("Failed to parse turbulence for texture: " + options);
            }
        }
    }
}
// Loads a Wavefront OBJ model from an input stream, including parsing any referenced MTL files
bool ObjLoader::load(std::istream& is) {
    std::string line;
    std::string mtlFile;

    MaterialGroup* currentGroup = nullptr;	
	//Create the default material
	if (materials.find(defaultMaterialName) == materials.end()) {
		Material defaultMaterial;
		defaultMaterial.name = defaultMaterialName;
		materials.insert(std::make_pair(defaultMaterialName, std::move(defaultMaterial)));
		auto [it, inserted] = materialGroups.try_emplace(defaultMaterialName, defaultMaterialName);
		currentGroup = &it->second;
	}
	

    while (std::getline(is, line)) {
        line = strip_comments(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string label;
        iss >> label;

        if (label == "v") {
            float x, y, z;
            if (iss >> x >> y >> z) {
                vertices.emplace_back(Vector3f{x, y, z});
            } else {
                throw std::runtime_error("Failed to parse vertex: " + line);
            }
        } else if (label == "vn") {
            float x, y, z;
            if (iss >> x >> y >> z) {
                normals.emplace_back(Vector3f{x, y, z});
            } else {
                throw std::runtime_error("Failed to parse normal: " + line);
            }
        } else if (label == "vt") {
            float u, v;
            if (iss >> u >> v) {
                texcoords.emplace_back(Vector2f{u, v});
            } else {
                throw std::runtime_error("Failed to parse texture coordinate: " + line);
            }
        } else if (label == "f") {
            std::string faceData;
            std::string allFaceData;
            while (iss >> faceData) {
                allFaceData += faceData + " ";
            }
            try {
                Face face = parseFace(allFaceData);
                faces.push_back(face);
                if (currentGroup) {
                    currentGroup->faces.push_back(face);
                }
            }
            catch (const std::runtime_error& error) {
                std::cerr << "Error parsing face: " << error.what() << std::endl;
            }

        } else if (label == "usemtl") {
            std::string material;
            iss >> material;
            auto [it, inserted] = materialGroups.try_emplace(material, material);
            currentGroup = &it->second;
        } else if (label == "mtllib") {
            iss >> mtlFile;
            if (!mtlFile.empty()) {
                std::ifstream mtlStream(mtlFile);
                if (!mtlStream.is_open()) {
                    throw std::runtime_error("Could not open MTL file: " + mtlFile);
                }
                readMaterial(mtlStream);
            }
        }
    }
    return true;
}
// Reads material definitions from an input stream typically from an MTL file
bool ObjLoader::readMaterial(std::istream& is) {
    std::string line;
    Material currentMaterial;

    while (std::getline(is, line)) {
        line = strip_comments(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string label;
        iss >> label;

        if (label == "newmtl") {
            if (!currentMaterial.name.empty()) {
                // Use move semantics when inserting into the map
                materials.insert(std::make_pair(currentMaterial.name, std::move(currentMaterial)));
                currentMaterial = Material(); // Reset for the next material
            }
            iss >> currentMaterial.name;
            assert(!currentMaterial.name.empty() && "Material name cannot be empty");
        } else if (label == "Ka" || label == "Kd" || label == "Ks") {
            auto& vec = (label == "Ka" ? currentMaterial.ambient :
                        (label == "Kd" ? currentMaterial.diffuse : currentMaterial.specular));
            float x, y, z;
            if (iss >> x >> y >> z) {
                vec = std::make_unique<Vector3f>(Vector3f{x, y, z});
            } else {
                throw std::runtime_error("Failed to parse " + label + " for material: " + line);
            }
        } else if (label == "Ns") {
            float shininess;
            if (iss >> shininess) {
                currentMaterial.shininess = std::make_unique<float>(shininess);
            } else {
                throw std::runtime_error("Failed to parse shininess: " + line);
            }
        } else if (label == "map_Kd" || label == "map_Ka" || label == "map_Ks" || label == "map_Ns" || label == "map_d" || label == "map_Bump" || label == "bump") {
            std::unique_ptr<TextureMap>& texMap = (label == "map_Kd" ? currentMaterial.diffuseTexture :
                                                   (label == "map_Ka" ? currentMaterial.ambientTexture :
                                                   (label == "map_Ks" ? currentMaterial.specularTexture :
                                                   (label == "map_Ns" ? currentMaterial.specularHighlightTexture :
                                                   (label == "map_d" ? currentMaterial.alphaTexture :
                                                   currentMaterial.bumpMap)))));
            texMap = std::make_unique<TextureMap>();
            iss >> texMap->filename;
            std::string options;
            std::getline(iss, options);
            parseTextureOptions(*texMap, options);
        } else  if (label == "d") {
			float dissolve;
			if (iss >> dissolve) {
				currentMaterial.transmission = std::make_unique<float>(1.0f - dissolve); // transmission = 1 - d
			} else {
				throw std::runtime_error("Failed to parse dissolve: " + line);
			}
		} else if (label == "Tr") {
			float transparency;
			if (iss >> transparency) {
				currentMaterial.transmission = std::make_unique<float>(transparency); // transmission = Tr
			} else {
				throw std::runtime_error("Failed to parse transparency: " + line);
			}
        } else if (label == "Ni") {
            float opticalDensity;
            if (iss >> opticalDensity) {
                currentMaterial.opticalDensity = std::make_unique<float>(opticalDensity);
            } else {
                throw std::runtime_error("Failed to parse optical density: " + line);
            }
        }
    }

    // Insert the last material if there's one after the loop
    if (!currentMaterial.name.empty()) {
        materials.insert(std::make_pair(currentMaterial.name, std::move(currentMaterial)));
    }


    return true;
}

} // namespace wavefront