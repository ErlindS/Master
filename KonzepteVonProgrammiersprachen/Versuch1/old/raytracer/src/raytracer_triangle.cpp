#include "raytracer.h"
#include "kd_tree.h"
#include "brute_force.h"
#include <iostream>
#include "logger.h"
#include <fstream>
#include "statistics.h"
#include "argument_parser.h"

std::vector<Argument> arguments = {
          Argument({"-no_samples"}, false, ArgumentType::PREFIX_VALUE, std::optional<int>(1),
  		     "The number of rays generate per pixel"),
          Argument({"-width"}, false, ArgumentType::PREFIX_VALUE, std::optional<int>(800),
             "The number of horizontal pixels of the image"),
          Argument({"-height"}, false, ArgumentType::PREFIX_VALUE, std::optional<int>(600),
             "The number of vertical pixels of the image"),
          Argument({"-o"}, false, ArgumentType::PREFIX_VALUE, std::optional<std::string>("output.ppm"),
             "Specifies the file PPM name of the output image"),
          Argument({"-hit_buffer_is_on"}, false, ArgumentType::SINGLE_VALUE, std::optional<bool>(true),
             "Enables hit buffer"),
          Argument({"-shadow_buffer_is_on"}, false, ArgumentType::SINGLE_VALUE, std::optional<bool>(true),
             "Enables shadow buffer"),
		  Argument({"-gamma_value"}, false, ArgumentType::PREFIX_VALUE, std::optional<float>(2.2),
             "The gamma value for the final image processing")
};

void init(int argc, char *argv[] ) {
	
    Argument::parseArguments(arguments, argc, const_cast<char**>(argv));
	
	statistic::stat.get<int>("no_samples") = arguments[0].get<int>().value_or(1);
	statistic::stat.get<int>("width") = arguments[1].get<int>().value_or(800);
	statistic::stat.get<int>("height") = arguments[2].get<int>().value_or(600);
	statistic::stat.get<std::string>("ppm_file_name") = arguments[3].get<std::string>().value_or("output.ppm");
	statistic::stat.get<bool>("hit_buffer_is_on") =  arguments[4].get<bool>().value_or(true);
	statistic::stat.get<bool>("shadow_buffer_is_on") = arguments[5].get<bool>().value_or(true);
	statistic::stat.get<float>("gamma_value") = arguments[6].get<float>().value_or(2.2);
};

int main(int argc, char *argv[] ) {
    if (Argument::checkHelpFlag(argc, argv, arguments)) {
        return 0;  
    }
	
	init(argc, argv);
    // Camera setup
    Camera3f camera(Vector3f{0, 0, 4}, Vector3f{0, 0, -1}, Vector3f{0, 1, 0}, 45.0f * M_PI / 180.0f, 1.0f, "perspective");

   // Materials
    auto red_material = std::make_shared<Material<float, 3>>(Color<float>(1.0f, 0.0f, 0.0f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f);
    auto green_material = std::make_shared<Material<float, 3>>(Color<float>(0.0f, 1.0f, 0.0f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f);
    auto white_material = std::make_shared<Material<float, 3>>(Color<float>(1.0f, 1.0f, 1.0f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f);
    auto glass_material = std::make_shared<Material<float, 3>>(Color<float>(1.0f, 1.0f, 1.0f), 0.1f, 0.1f, 0.9f, 20.0f, 0.1f, 0.9f, 1.5f);
    auto mirror_material = std::make_shared<Material<float, 3>>(Color<float>(1.0f, 1.0f, 1.0f), 0.1f, 0.0f, 1.0f, 20.0f, 1.0f, 0.0f, 1.0f);
    auto orange_material = std::make_shared<Material<float, 3>>(Color<float>(1.0f, 0.5f, 0.0f), 0.1f, 0.9f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f);

    std::vector<std::shared_ptr<Triangle3f>> triangles;
    std::vector<Triangle3f> box;
	
	// Scene setup with Cornell Box (Walls)
    
	box = geom::create_prisma(Vector3f{-1, -1.1, 0}, Vector3f{2, 0, 0}, Vector3f{0, 0, -2}, Vector3f{0, 0.1, 0}, true, white_material); // bottom
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }
	box = geom::create_prisma(Vector3f{-1, 1.0, 0}, Vector3f{2, 0, 0}, Vector3f{0, 0, -2}, Vector3f{0, 0.1, 0}, true, white_material); // top
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }
	box = geom::create_prisma(Vector3f{-1, -1.1, -2}, Vector3f{2, 0, 0}, Vector3f{0, 0, -0.1}, Vector3f{0, 2.2, 0}, true, white_material); // back
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }
	box = geom::create_prisma(Vector3f{-1.1, -1.1, 0}, Vector3f{0.1, 0, 0}, Vector3f{0, 0, -2}, Vector3f{0, 2.2, 0}, true, green_material); // left
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }
	box = geom::create_prisma(Vector3f{1, -1.1, 0}, Vector3f{0.1, 0, 0}, Vector3f{0, 0, -2}, Vector3f{0, 2.2, 0}, true, red_material); // right
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }

    // lets put some objects into the box
	box = geom::create_prisma(Vector3f{-0.25, -1.0001, 0}, Vector3f{0.4, 0.0, -0.1}, Vector3f{0, 0, -0.5}, Vector3f{-0.2, 0.6, 0}, true, orange_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }
	box = geom::create_prisma(Vector3f{0.25, -1.0001, -0.5}, Vector3f{0.4, 0.0, -0.1}, Vector3f{-0.2, 0, -0.4}, Vector3f{0.0, 0.8, 0}, true, mirror_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle)); 
    }
	box = geom::create_prisma(Vector3f{-0.7, -1.0001, -0.5}, Vector3f{0.4, 0.0, 0.0}, Vector3f{0.0, 0, -0.4}, Vector3f{0.0, 0.7, 0}, true, glass_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }
	box = geom::create_prisma(Vector3f{0.7, -1.0001, -0.1}, Vector3f{0.2, 0.0, 0.0}, Vector3f{0, 0, -0.2}, Vector3f{0.0, 0.2, 0}, true, green_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }
		box = geom::create_prisma(Vector3f{0.5, -1.0001, -0.1}, Vector3f{0.1, 0.0, 0.0}, Vector3f{0, 0, -0.1}, Vector3f{0.0, 0.7, 0}, true, red_material);
    for (const auto& triangle : box) {
         triangles.push_back(std::make_shared<Triangle3f>(triangle));
    }
	statistic::stat.set<int>("no_triangles", triangles.size());
    // Light
    std::vector<Light3f> lights = {
		Light3f(Vector3f{-0.5, 0.95, -1}, Vector3f{-1, -1, -1}, Color<float>{1.0f, 1.0f, 1.0f}),
		Light3f(Vector3f{ 0.5, 0.95, -1}, Vector3f{-1, -1, -1}, Color<float>{1.0f, 1.0f, 1.0f})
	};

	statistic::stat.set<int>("no_lights", lights.size());
    
    // Image setup
    Image image(statistic::stat.get<int>("width"), statistic::stat.get<int>("height"), 255);

    auto structure = std::make_shared<KDTree3fTriangle>();
	structure->build(triangles);
	

    Raytracer<float, Triangle3f> raytracer(structure, statistic::stat.get<int>("no_samples"));
	

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
