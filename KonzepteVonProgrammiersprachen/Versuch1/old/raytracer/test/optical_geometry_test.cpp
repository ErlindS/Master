#include <gtest/gtest.h>
#include "geometry.h"
#include "optical_geometry.h"
#include <random>

/**
 * @file optical_geometry_test.cpp
 * @brief Contains unit tests for the OpticalGeometry class using Google Test.
 */

/**
 * @brief Test the reflection method of OpticalGeometry.
 * 
 * Checks if the reflection is correctly calculated for a simple case where 
 * the incident ray is perpendicular to the surface.
 */
TEST(OpticalGeometryTest, Reflect1) {
	Vector3f normal = Vector3f{0, 0, -1};
	Ray3f incidentRay = Ray3f{ Vector3f{0, 0, -1}, Vector3f{0, 0, 1} };
    Vector3f reflected = OpticalGeometry3f::reflect(incidentRay, normal);
    EXPECT_FLOAT_EQ(reflected[0], 0);
    EXPECT_FLOAT_EQ(reflected[1], 0);
    EXPECT_FLOAT_EQ(reflected[2], -1); 
}

/**
 * @brief Test the reflection method of OpticalGeometry.
 * 
 * Checks if the reflection is correctly calculated for a simple case where 
 * the incident ray is perpendicular to the surface.
 */
TEST(OpticalGeometryTest, Reflect2) {
	Vector3f normal = Vector3f{0, 1, 0};
	Ray3f incidentRay = Ray3f{ Vector3f{-1, -1, 0}, Vector3f{1, -1, 0}.normalized() };
    Vector3f reflected = OpticalGeometry3f::reflect(incidentRay, normal);
	Vector3f expected = Vector3f{1, 1, 0}.normalized();
    EXPECT_FLOAT_EQ(reflected[0], expected[0] );
    EXPECT_FLOAT_EQ(reflected[1], expected[1]);
    EXPECT_FLOAT_EQ(reflected[2], expected[2]);
}


/**
 * @brief Test the Schlick-Fresnel for extrem cases.
 * 
 * Checks if the Schlick-Fresnel approximation  is correctly calculated for the simple cases.
 */
TEST(OpticalGeometryTest, SchlickFresnel) {
    OpticalGeometry<float, 3> og;
    EXPECT_NEAR(og.schlickFresnel(1.0f, 1.0f, 1.5f), 0.04f, 0.001f); // Normal incidence
    EXPECT_NEAR(og.schlickFresnel(0.0f, 1.0f, 1.5f), 1.0f, 0.01f); // Grazing angle
}


