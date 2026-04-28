#ifndef OBJ_TO_TRIANGLE_H
#define OBJ_TO_TRIANGLE_H

#include <vector>
#include <iostream>
#include <memory>
#include "obj_loader.h"
#include "geometry.h"
#include "shading_algorithms.h"

std::vector<std::shared_ptr<TriangleWithNormals3f>> setup_triangles(wavefront::ObjLoader & objLoader) {
	std::vector<std::shared_ptr<TriangleWithNormals3f>> triangles;
    const auto& vertices = objLoader.getVertices();
    const auto& normals = objLoader.getNormals();
    const auto& materials = objLoader.getMaterials();

    for (const auto& [materialName, materialGroup] : objLoader.getMaterialGroups()) {
		std::shared_ptr<Material<float, 3>> material;

        // Find the material in the loaded materials
        auto it = materials.find(materialName);
		
        if (it != materials.end()) {
        
            const auto& loadedMaterial = it->second;
			Color<float> color{ 0.0f, 0.0f, 0.0f };
			float ka = 0.1f;
			float kd = 0.9f;
			float ks = 0.0f;
			float shininess = 20.0f;
			float reflectivity = 0.0f;
			float transmission = 0.0f;
			float opticalDensity = 1.0f;
			float emission = 0.0f;

			if (loadedMaterial.ambient != nullptr) {
				Vector3f ambient_vector = *loadedMaterial.ambient;
				ka = (ambient_vector[0] + ambient_vector[1] + ambient_vector[2]) / 3;
			}
			
			if (loadedMaterial.diffuse != nullptr) {
				Vector3f diffuse_vector = *loadedMaterial.diffuse;
				kd = (diffuse_vector[0] + diffuse_vector[1] + diffuse_vector[2]) / 3;
				color = Color<float>(diffuse_vector[0], diffuse_vector[1], diffuse_vector[2]);
			}
			if (loadedMaterial.specular != nullptr) {
				Vector3f specular_vector = *loadedMaterial.specular;
				reflectivity = (specular_vector[0] + specular_vector[1] + specular_vector[2]) / 3;
			}
			if (loadedMaterial.shininess != nullptr) {
				shininess = *loadedMaterial.shininess;
			}
			if (loadedMaterial.transmission != nullptr) {
				transmission = *loadedMaterial.transmission;
			}
			if (loadedMaterial.opticalDensity != nullptr) {
				opticalDensity = *loadedMaterial.opticalDensity;
			}
			
            // Create a Material3f using the loaded material properties
            material = std::make_shared<Material<float, 3>>(color, ka, kd, ks, shininess, reflectivity, transmission, opticalDensity, emission);		
        } else {
            // If the material is not found, create a default material
            material = std::make_shared<Material<float, 3>>(Color<float>(0.5f, 0.5f, 0.5f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f);
            std::cerr << "Warning: Material '" << materialName << "' not found, using default material." << std::endl;
        }

       // Iterate over the faces in the group and create TriangleWithNormals
        for (const auto& face : materialGroup.faces) {

            if (face.vertexIndices.size() != 3) {
                // Handle non-triangular faces. This version simply skips them.
                std::cerr << "Warning: Skipping non-triangular face (vertex count = " << face.vertexIndices.size() << ")" << std::endl;
                continue;
            }

			// Get the verticies from the face.
            Vector3f v0 = vertices[face.vertexIndices[0]];
            Vector3f v1 = vertices[face.vertexIndices[1]];
			Vector3f v2 = vertices[face.vertexIndices[2]];

            Vector3f n0, n1, n2;

			//Checking if face has normals.
			if (face.normalIndices.size() == 3 && face.normalIndices[0] >= 0 && face.normalIndices[1] >= 0 && face.normalIndices[2] >= 0) {
			    n0 = normals[face.normalIndices[0]];
				n1 = normals[face.normalIndices[1]];
				n2 = normals[face.normalIndices[2]];
				Vector3f calculatedNormal = ((v1 - v0).cross(v2 - v0)).normalized();

			    float dotProduct = calculatedNormal.dot(n0);
			
			     // Wenn das Dot-Produkt negativ ist, kehre die Reihenfolge der Eckpunkte um, um die Normale umzukehren
			    if (dotProduct < 0) {
				    //Kehrt die Reihenfolge der Vertices um
				    std::swap(v1, v2);

				    n0 = normals[face.normalIndices[0]];
				    n1 = normals[face.normalIndices[1]];
				    n2 = normals[face.normalIndices[2]];				
			    }
			} else {
				Vector3f generatedNormal = ((v1 - v0).cross(v2 - v0)).normalized();
				n0 = generatedNormal;
				n1 = generatedNormal;
				n2 = generatedNormal;
			}
 
            triangles.push_back(std::make_shared<TriangleWithNormals3f>(v0, v1, v2, n0, n1, n2, true, material));

        }
    }

	return triangles;
}

#endif