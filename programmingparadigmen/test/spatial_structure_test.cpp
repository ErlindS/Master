#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "spatial_structure.h"
#include "brute_force.h"
#include "kd_tree.h"
#include "vector.h"
#include "geometry.h"
#include <iostream>
#include <random>

#include "statistics.h"

using namespace math;
using namespace geom;

// Helper function to create a sphere with a given center and radius
std::shared_ptr<Sphere3f> create_sphere(Vector3f center, float radius) {
    return std::make_shared<Sphere3f>(center, radius);
}
 
/**
 * @class SpatialStructureTest
 * @brief A test fixture class for KDTree tests, setting up a consistent test environment.
 */
template <typename T>
class SpatialStructureTest : public ::testing::Test {
protected:
    T spatial_structure;
    std::vector<std::shared_ptr<Sphere3f>> all_spheres;

    void SetUp() override {
        all_spheres = {
            std::make_shared<Sphere3f>(Vector3f{0, 0, 0}, 1.0f),
            std::make_shared<Sphere3f>(Vector3f{5, 0, 0}, 1.0f),
            std::make_shared<Sphere3f>(Vector3f{0, 5, 0}, 1.0f),
            std::make_shared<Sphere3f>(Vector3f{0, 0, 5}, 1.0f),
            std::make_shared<Sphere3f>(Vector3f{2, 2, 2}, 1.0f)
        };
	    statistic::stat.get<bool>("kd_tree_hit_buffer_is_on") = true; 
        spatial_structure.build(all_spheres);  
    }

Vector3f randomVector(float min, float max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return Vector3f{dis(gen), dis(gen), dis(gen)};
}


};


    void EXPECT_VECTOR_EQ(const Vector3f& expected, const Vector3f& actual, float delta = 1e-5f) {
        EXPECT_NEAR(expected[0], actual[0], delta);
        EXPECT_NEAR(expected[1], actual[1], delta);
        EXPECT_NEAR(expected[2], actual[2], delta);
    }
	
TYPED_TEST_SUITE_P(SpatialStructureTest);


TYPED_TEST_P(SpatialStructureTest, FindNearestIntersection) {
    // Ray starting just outside the sphere at (0,0,0) but close to it, pointing towards the sphere at (5,0,0)
    Ray3f ray(Vector3f{1.1f, 0, 0}, Vector3f{1, 0, 0}); // Starting just outside the sphere at origin
    auto intersection = this->spatial_structure.findNearestIntersection(ray);

    // Check if the hit is detected and the distance matches the expected value for the sphere at (5,0,0)
    EXPECT_TRUE(intersection.hit);
    EXPECT_FLOAT_EQ(intersection.t, 2.9f); // Distance from 1.1 to 5 (sphere at (5,0,0)) minus radius
}

				 

TYPED_TEST_P(SpatialStructureTest, FindNearestIntersection1) {
    Ray3f ray(Vector3f{0, 0, -4}, Vector3f{0, 0, 1}.normalized()); // Starting just outside the sphere at origin
    auto intersection = this->spatial_structure.findNearestIntersection(ray);

    // Check if the hit is detected and the distance matches the expected value for the sphere at (5,0,0)
    EXPECT_TRUE(intersection.hit);
    EXPECT_FLOAT_EQ(intersection.t, 3.0f); // Distance from 1.1 to 5 (sphere at (5,0,0)) minus radius
}



TYPED_TEST_P(SpatialStructureTest, SingleObjectIntersection) {
    std::vector<std::shared_ptr<Sphere3f>> spheres;
    spheres.push_back(std::make_shared<Sphere3f>(Vector3f{0, 0, 0}, 1.0f)); // Sphere at origin with radius 1
    
    this->spatial_structure.build(spheres);

    // Ray starting from (-2,0,0) pointing towards (2,0,0), should intersect the sphere
    Ray3f ray(Vector3f{-2, 0, 0}, Vector3f{1, 0, 0}.normalized());
    auto intersection = this->spatial_structure.findNearestIntersection(ray);

    EXPECT_TRUE(intersection.hit);
    EXPECT_NEAR(intersection.t, 1.0f, 1e-5f); // Distance from -2 to -1 (surface of sphere)

    // Ray starting from (2,0,0) pointing towards (1,0,0), should not intersect since outside the sphere's radius
    Ray3f ray_no_hit(Vector3f{2, 0, 0}, Vector3f{1, 0, 0}.normalized());
    intersection = this->spatial_structure.findNearestIntersection(ray_no_hit);

    EXPECT_FALSE(intersection.hit);
}



TYPED_TEST_P(SpatialStructureTest, CompareBruteForceWithKDTree) {
    // Ray starting from origin towards (1, 1, 1)
    Ray3f ray(Vector3f{0, 0, -4}, Vector3f{0, 0, 1}.normalized());


    auto intersection = this->spatial_structure.findNearestIntersection(ray);

    // Brute force intersection using the stored spheres
    auto bruteForceIntersection = KDTree3fSphere::bruteForceNearestIntersection(ray, this->all_spheres);

    // Check if both methods agree on the intersection
    EXPECT_EQ(intersection.hit, bruteForceIntersection.hit);
    
    if (intersection.hit && bruteForceIntersection.hit) {
        EXPECT_FLOAT_EQ(intersection.t, bruteForceIntersection.t);
        EXPECT_VECTOR_EQ(intersection.normal, bruteForceIntersection.normal, 1e-5f) ;
    }
}

TYPED_TEST_P(SpatialStructureTest, RandomRayCompareBruteForceWithKDTree) {
    // Generate a random ray ensuring the origin is outside the sphere at (0,0,0) with radius 7
    Vector3f origin;
    do {
        origin = this->randomVector(-10.0f, 10.0f); // Generate until outside of radius 7 sphere
    } while (origin.length() <= 7.0f);

    Vector3f direction = this->randomVector(-1.0f, 1.0f).normalized();
    Ray3f ray(origin, direction);

    auto intersection = this->spatial_structure.findNearestIntersection(ray);

    // Brute force intersection using the stored spheres
    auto bruteForceIntersection = KDTree3fSphere::bruteForceNearestIntersection(ray, this->all_spheres);

    // Check if both methods agree on the intersection
    EXPECT_EQ(intersection.hit, bruteForceIntersection.hit);
    
    if (intersection.hit && bruteForceIntersection.hit) {
        EXPECT_NEAR(intersection.t, bruteForceIntersection.t, 1e-5f);
        EXPECT_VECTOR_EQ(intersection.normal, bruteForceIntersection.normal, 1e-5f);
    }
}

TYPED_TEST_P(SpatialStructureTest, GivenRayRandomSpheresCompareBruteForceWithKDTree) {
	 // Generate random spheres
     std::random_device rd;
     std::mt19937 gen(rd());
     std::uniform_real_distribution<float> dis(-10.0f, 10.0f);
     std::uniform_real_distribution<float> radius_dis(0.5f, 2.0f);  // Random radii between 0.5 and 2

    std::vector<std::shared_ptr<Sphere3f>> spheres;

     for (int i = 0; i < 100; ++i) { // Generating 100 random spheres
         Vector3f center = Vector3f{dis(gen), dis(gen), dis(gen)};
         float radius = radius_dis(gen);
         spheres.push_back(std::make_shared<Sphere3f>(center, radius));
     }

    this->spatial_structure.build(spheres);  
		
    // Use a specific ray for testing
    Ray3f ray(Vector3f{-20, 0, 0}, Vector3f{-1, 0, 0}.normalized());


    auto intersection = this->spatial_structure.findNearestIntersection(ray);

    // Brute force intersection using the stored spheres
    auto bruteForceIntersection = KDTree3fSphere::bruteForceNearestIntersection(ray, this->all_spheres);

    // Check if both methods agree on the intersection
    EXPECT_EQ(intersection.hit, bruteForceIntersection.hit);
    
    if (intersection.hit && bruteForceIntersection.hit) {
        EXPECT_NEAR(intersection.t, bruteForceIntersection.t, 1e-5f);
        EXPECT_VECTOR_EQ(intersection.normal, bruteForceIntersection.normal, 1e-5f);
    }
}


TYPED_TEST_P(SpatialStructureTest, GivenRayRandomSpheresRandomRayCompareBruteForceWithKDTree) {
	 // Generate random spheres
     std::random_device rd;
     std::mt19937 gen(rd());
     std::uniform_real_distribution<float> dis(-10.0f, 10.0f);
     std::uniform_real_distribution<float> radius_dis(0.5f, 2.0f);  // Random radii between 0.5 and 2

    std::vector<std::shared_ptr<Sphere3f>> spheres;

     for (int i = 0; i < 100; ++i) { // Generating 100 random spheres
         Vector3f center = Vector3f{dis(gen), dis(gen), dis(gen)};
         float radius = radius_dis(gen);
         spheres.push_back(std::make_shared<Sphere3f>(center, radius));
     }
	
    this->spatial_structure.build(spheres);  // Building the KD-Tree with random spheres
	
  for (int i = 0; i < 100; ++i) {		
    Vector3f origin = this->randomVector(-30.0f, 30.0f); // Generate until outside of scene    
    Vector3f direction = this->randomVector(-1.0f, 1.0f).normalized();
    Ray3f ray(origin, direction);


    auto intersection = this->spatial_structure.findNearestIntersection(ray);

    // Brute force intersection using the stored spheres
    auto bruteForceIntersection = KDTree3fSphere::bruteForceNearestIntersection(ray, spheres);


    // Check if both methods agree on the intersection
    EXPECT_EQ(intersection.hit, bruteForceIntersection.hit);
    if (intersection.hit && bruteForceIntersection.hit) {
		EXPECT_NE(intersection.object, nullptr);
		EXPECT_NE(bruteForceIntersection.object, nullptr);
		EXPECT_NEAR(intersection.t, bruteForceIntersection.t, 1e-5f);
		EXPECT_GT(intersection.t, -1e-5f);
		EXPECT_GT(bruteForceIntersection.t, -1e-5f);
        EXPECT_VECTOR_EQ(intersection.normal, bruteForceIntersection.normal, 1e-5f);	
        if ( std::abs(intersection.t - bruteForceIntersection.t) > 1e-5f ) {			
		    EXPECT_EQ(bruteForceIntersection.object.get(), intersection.object.get());
		}
    } 
  }
}


REGISTER_TYPED_TEST_SUITE_P(SpatialStructureTest, 
    FindNearestIntersection, 
    FindNearestIntersection1,
	SingleObjectIntersection,
	CompareBruteForceWithKDTree,
	RandomRayCompareBruteForceWithKDTree,
	GivenRayRandomSpheresCompareBruteForceWithKDTree,
	GivenRayRandomSpheresRandomRayCompareBruteForceWithKDTree
);


using MyTypes = ::testing::Types<BruteForce3fSphere, KDTree3fSphere>;
// Instantiate the test suite for specific types
INSTANTIATE_TYPED_TEST_SUITE_P(SpatialStructure, 
    SpatialStructureTest, 
    MyTypes);
