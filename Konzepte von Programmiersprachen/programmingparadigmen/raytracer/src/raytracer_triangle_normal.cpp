// main.cpp
#include "raytracer.h"
#include "path_tracer.h"
#include "kd_tree.h"
#include "brute_force.h"
#include <iostream>
#include "logger.h"
#include <fstream>
#include "statistics.h"
#include "argument_parser.h"
#include "obj_loader.h"
#include "obj_to_triangle.h" 


std::vector<Argument> arguments = {
          Argument({"-no_samples"}, false, ArgumentType::PREFIX_VALUE, std::optional<int>(1),
  		     "The number of rays generate per pixel"),
          Argument({"-width"}, false, ArgumentType::PREFIX_VALUE, std::optional<int>(800),
             "The number of horizontal pixels of the image"),
          Argument({"-height"}, false, ArgumentType::PREFIX_VALUE, std::optional<int>(600),
             "The number of vertical pixels of the image"),
          Argument({"-o"}, false, ArgumentType::PREFIX_VALUE, std::optional<std::string>("output.ppm"),
             "Specifies the file PPM name of the output image"),
          Argument({"-hit_buffer_is_on"}, false, ArgumentType::SINGLE_VALUE, std::optional<bool>(false),
             "Enables hit buffer"),
          Argument({"-shadow_buffer_is_on"}, false, ArgumentType::SINGLE_VALUE, std::optional<bool>(false),
             "Enables shadow buffer"),
		  Argument({"-gamma_value"}, false, ArgumentType::PREFIX_VALUE, std::optional<float>(2.2),
             "The gamma value for the final image processing"),
		  Argument({"-wavefront"}, false, ArgumentType::PREFIX_VALUE, std::optional<std::string>("teapot_n_glass.obj"),
             "The wavefront input file.")
};

void init(int argc, char *argv[] ) {

    Argument::parseArguments(arguments, argc, const_cast<char**>(argv));

	statistic::stat.get<int>("no_samples") = arguments[0].get<int>().value_or(1);
	statistic::stat.get<int>("width") = arguments[1].get<int>().value_or(800);
	statistic::stat.get<int>("height") = arguments[2].get<int>().value_or(600);
	statistic::stat.get<std::string>("ppm_file_name") = arguments[3].get<std::string>().value_or("output.ppm");
	statistic::stat.hit_buffer_is_on =  arguments[4].get<bool>().value_or(false);
	statistic::stat.shadow_buffer_is_on = arguments[5].get<bool>().value_or(false);
	statistic::stat.gamma_value = arguments[6].get<float>().value_or(2.2);
	statistic::stat.get<std::string>("wavefront") = arguments[7].get<std::string>().value_or("teapot.obj");
	
	 
};

template<typename T, size_t Dim>
std::pair<Vector<T, Dim>, Vector<T, Dim>> calculateBoundingBox(const std::vector<std::shared_ptr<geom::TriangleWithNormals<T, Dim>>>& triangles) {
    if (triangles.empty()) {
        throw std::invalid_argument("Cannot calculate bounding box for an empty set of triangles.");
    }

    Vector<T, Dim> min = triangles[0]->vertices[0];
    Vector<T, Dim> max = triangles[0]->vertices[0];

    for (const auto& triangle : triangles) {
        for (int i = 0; i < 3; ++i) {
            for (size_t j = 0; j < Dim; ++j) {
                min[j] = std::min(min[j], triangle->vertices[i][j]);
                max[j] = std::max(max[j], triangle->vertices[i][j]);
            }
        }
    }
    return std::make_pair(min, max);
}

template<typename T, size_t Dim>
Camera<T> setupCamera(const Vector<T, Dim>& min, const Vector<T, Dim>& max, float fov_degrees, float aspectRatio, float zoom = 1.0f) {
    //Calculate center of the bounding box
    Vector<T, Dim> center = (min + max) * 0.5f;

    //Calculate diagonal of the bounding box
    Vector<T, Dim> diagonal = max - min;
    T radius = diagonal.length() * 0.5f;

    //Calculate camera distance based on field of view
    T fov = fov_degrees * M_PI / 180.0f;
    T distance = radius / std::tan(fov * 0.5f);

    //Set camera position
    Vector<T, Dim> cameraPosition = center - Vector<T, Dim>{0, 0, zoom * distance};

    //Set up direction and up vectors
    Vector<T, Dim> lookAt = center;
    Vector<T, Dim> upVector = Vector<T, Dim>{0, 1, 0};

    return Camera<T>(cameraPosition, lookAt, upVector, fov, aspectRatio, "perspective");
}

std::vector<std::shared_ptr<TriangleWithNormals3f>> create_scene_1() {
	std::cout << "reading " << statistic::stat.get<std::string>("wavefront") << std::endl;
    
	std::ifstream objFile(statistic::stat.get<std::string>("wavefront")); 
    if (!objFile.is_open()) {
        std::cerr << "Error: Could not open OBJ file." << std::endl;
        throw 1;
    }
	
    wavefront::ObjLoader objLoader(objFile);
	std::cout << statistic::stat.get<std::string>("wavefront") << " parsed" << std::endl;
	
	std::vector<std::shared_ptr<TriangleWithNormals3f>> triangles = setup_triangles(objLoader);

    return triangles;
}

std::vector<std::shared_ptr<TriangleWithNormals3f>> create_scene_2() {
    std::vector<std::shared_ptr<TriangleWithNormals3f>> triangles;
    
    // Bounding box similar to scene 1
    Vector3f min{-4.52149f, -0.089041f, -4.52149f};
    Vector3f max{4.52149f, 3.1473f, 4.52149f};
    
    // Random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist_x(min[0], max[0]);
    std::uniform_real_distribution<float> dist_y(min[1], max[1]);
    std::uniform_real_distribution<float> dist_z(min[2], max[2]);
    
    // Material for triangles (similar to Cornell box grey material)
    auto material = std::make_shared<Material<float, 3>>(
        Color<float>(0.9f, 0.9f, 0.9f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f, 0.0f
    );
    
    // Generate 100 random triangles (adjust this number as needed)
    const int num_triangles = 1000000;
    for (int i = 0; i < num_triangles; ++i) {
        // Generate three random vertices
        Vector3f v1{dist_x(gen), dist_y(gen), dist_z(gen)};
        Vector3f v2{dist_x(gen), dist_y(gen), dist_z(gen)};
        Vector3f v3{dist_x(gen), dist_y(gen), dist_z(gen)};
        
        // Calculate face normal
        Vector3f edge1 = v2 - v1;
        Vector3f edge2 = v3 - v1;
        Vector3f normal = edge1.cross(edge2).normalized();
        
        // Create triangle with same normal for all vertices
        auto triangle = std::make_shared<TriangleWithNormals3f>(
            v1, v2, v3,
            normal, normal, normal,
            true, // counter-clockwise
            material
        );
        
        triangles.push_back(triangle);
    }
    
    return triangles;
}

void add_cornell_box(std::vector<std::shared_ptr<TriangleWithNormals3f>>  & triangles) {
	auto emitting_material = std::make_shared<Material<float, 3>>(Color<float>(1.0f, 1.0f, 1.0f), 0.0f, 0.0f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    auto grey_material = std::make_shared<Material<float, 3>>(Color<float>(0.9f, 0.9f, 0.9f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    auto red_material = std::make_shared<Material<float, 3>>(Color<float>(0.9f, 0.0f, 0.0f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    auto green_material = std::make_shared<Material<float, 3>>(Color<float>(0.0f, 0.9f, 0.0f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   
	// bottom
	auto box = geom::create_prisma(Vector3f{-20.0, -2.0, -20.0}, Vector3f{40, 0.0, 0.0}, Vector3f{0, 0, 40}, Vector3f{0.0, 1, 0}, true, grey_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<TriangleWithNormals3f>(triangle));
    }

	// top
	box = geom::create_prisma(Vector3f{-20.0, 20.0, -20.0}, Vector3f{40, 0.0, 0.0}, Vector3f{0, 0, 40}, Vector3f{0.0, 1, 0}, true, grey_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<TriangleWithNormals3f>(triangle));
    }
	
	// back
	box = geom::create_prisma(Vector3f{-20.0, -2.0, 20.0}, Vector3f{40, 0.0, 0.0}, Vector3f{0, 0, 2}, Vector3f{0.0, 40, 0}, true, grey_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<TriangleWithNormals3f>(triangle));
    }

    // left
	box = geom::create_prisma(Vector3f{-22.0, -2.0, 20.0}, Vector3f{2, 0.0, 0.0}, Vector3f{0, 0, 40}, Vector3f{0.0, 40, 0}, true, red_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<TriangleWithNormals3f>(triangle));
    }
	
	// right
	box = geom::create_prisma(Vector3f{20.0, -2.0, 20.0}, Vector3f{2, 0.0, 0.0}, Vector3f{0, 0, 40}, Vector3f{0.0, 40, 0}, true, green_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<TriangleWithNormals3f>(triangle));
    }
	
}

int main(int argc, char *argv[] ) {
    if (Argument::checkHelpFlag(argc, argv, arguments)) {
        return 0;
    }

	init(argc, argv);
    Argument::warnUnrecognized(argc, argv, arguments);
	
	// Using the unoptimized default implementation
	// Set currentIntersectionPolicy to an instance of an optimized algorithm
	TriangleIntersectionPolicy3f intersectionPolicy;
	TriangleIntersectionPolicy3f::currentIntersectionPolicy = &intersectionPolicy;
	    
    std::vector<Light3f> lights = { // for whitted style only
        Light3f(Vector3f{5, 5, -5}, Vector3f{1.0f, 1.0f, 1.0f}, Color<float>{1.0f, 1.0f, 1.0f}),
		Light3f(Vector3f{5, 5, 5}, Vector3f{1.0f, 1.0f, 1.0f}, Color<float>{1.0f, 1.0f, 1.0f})
    };

    auto triangles = create_scene_1();

    // Image setup
    Image image(statistic::stat.get<int>("width"), statistic::stat.get<int>("height"), 255);

	std::pair<Vector3f, Vector3f> boundingBox = calculateBoundingBox(triangles);
    Vector3f min = boundingBox.first;
    Vector3f max = boundingBox.second;
	
    float aspectRatio = static_cast<float>(statistic::stat.get<int>("width")) / statistic::stat.get<int>("height");
	Camera3f camera = setupCamera<float,3>(min , max , 45.0f, aspectRatio);

	add_cornell_box(triangles);
	
	std::cout << "no of triangles: " << triangles.size() << ", bytes used " << sizeof(TriangleWithNormals3f) * triangles.size() << std::endl;
	
    //auto structure =  structure = std::make_shared<KDTree<float, 3, TriangleWithNormals3f>>();
	auto structure = std::make_shared<BruteForce<float, 3, TriangleWithNormals3f>>();
	structure->build(triangles);

    Raytracer<float, TriangleWithNormals3f> raytracer(structure, statistic::stat.get<int>("no_samples"), 10 );
    
	statistic::stat.printInfo(std::cout);

    raytracer.render(camera, lights, image);
	{
	  std::string filename = statistic::stat.get<std::string>("ppm_file_name");
	  logging::logger << logging::Logger<true>::LogLevel::INFO << "writing image to file '" << filename << "'." << std::endl;
	  std::ofstream file(filename);
	  if (file) {
        image.write_ppm(file);
	  } else {
		logging::logger << logging::Logger<true>::LogLevel::WARNING << "PPM output file could not be opened." << std::endl;
	  }
	}

	statistic::stat.printInfo(std::cout);
    return 0;
}