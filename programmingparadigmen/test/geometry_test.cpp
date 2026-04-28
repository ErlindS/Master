#include <gtest/gtest.h>
#include "geometry.h"
#include <iostream>


using namespace geom;

TEST(GeometryTest, SimpleRaySphereIntersection) {
    Sphere3f sphere(Vector3f{0, 0, 0}, 1.0f);
    Ray3f ray(Vector3f{2, 0, 0}, Vector3f{-1, 0, 0});
    auto result = sphere.intersect(ray);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5);
}


TEST(GeometryTest, TriangleWithNormalsIntersection3D) {
	// Define a triangle with vertices and normals
    Vector3f v1{0, 0, 0}, v2{1, 0, 0}, v3{0, 1, 0};
    Vector3f n1{0, 0, 1}, n2{0, 0, 1}, n3{0, 0, 1}; // Flat surface for simplicity
    
    TriangleWithNormals3f triangle(v1, v2, v3, n1, n2, n3);

    // Define a ray that should intersect the triangle
    Ray3f ray(Vector3f{0.5, 0.5, -1}, Vector3f{0, 0, 1}); // Ray direction towards the triangle

    Intersection<float, 3> result = triangle.intersect(ray);
    
    // Check if intersection occurred
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5);  // Assuming the ray hits at z = 0
    
    // Check if the normal is correctly calculated (for this flat triangle, all normals should contribute equally)
    EXPECT_NEAR(result.normal[0], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[2], 1.0f, 1e-5);
	
    // Check UV coordinates for barycentric interpolation
    EXPECT_NEAR(result.u, 0.5f, 1e-5);  // Midpoint should be around 0.5 for both U and V
    EXPECT_NEAR(result.v, 0.5f, 1e-5);
}

TEST(GeometryTest, TriangleWithNormalsNoIntersection3D) {
    // Define a triangle with vertices and normals
    Vector3f v1{0, 0, 0}, v2{1, 0, 0}, v3{0, 1, 0};
    Vector3f n1{0, 0, 1}, n2{0, 0, 1}, n3{0, 0, 1};
    
    TriangleWithNormals3f triangle(v1, v2, v3, n1, n2, n3);

    // Ray that should not intersect
    Ray3f ray(Vector3f{2, 2, 0}, Vector3f{0, 0, 1}); // Ray direction parallel to the triangle's plane

    Intersection<float, 3> result = triangle.intersect(ray);
    
    EXPECT_FALSE(result.hit);
    EXPECT_NEAR(result.t, 0.0f, 1e-5);  // No intersection, t should be 0 or negative
}

TEST(GeometryTest, TriangleWithNormalsCurvedSurface3D) {
    // Define a triangle with vertices and normals simulating a curved surface
    Vector3f v1{0, 0, 0}, v2{1, 0, 0}, v3{0, 1, 0};
    Vector3f n1{0, 0, 1}, n2{1, 0, 0}, n3{0, 1, 0}; // Normals pointing in different directions
    
    TriangleWithNormals3f triangle(v1, v2, v3, n1.normalized(), n2.normalized(), n3.normalized());

    // Ray towards an edge of the triangle
    Ray3f ray(Vector3f{0.5, 0.5, -1}, Vector3f{0, 0, 1});

    Intersection<float, 3> result = triangle.intersect(ray);
    
    EXPECT_TRUE(result.hit);

    // Calculate the expected normal after interpolation and normalization
    Vector3f expected_normal = (0.5f * n2 + 0.5f * n3).normalized();
    std::cout << "result = " << result.normal << std::endl;
    EXPECT_NEAR(result.normal[0], expected_normal[0], 1e-5);
    EXPECT_NEAR(result.normal[1], expected_normal[1], 1e-5);
    EXPECT_NEAR(result.normal[2], expected_normal[2], 1e-5);

    EXPECT_NEAR(result.u, 0.5f, 1e-5);
    EXPECT_NEAR(result.v, 0.5f, 1e-5);
}

// Test cases for Triangle intersect method
TEST(GeometryTest, TriangleIntersection3D) {
    Vector3f v1{0, 0, 0}, v2{1, 0, 0}, v3{0, 1, 0};
    Triangle3f triangle(v1, v2, v3, true); // Counterclockwise

    Ray3f ray(Vector3f{0.5, 0.5, -1}, Vector3f{0, 0, 1}); // Ray towards the center

    Intersection<float, 3> result = triangle.intersect(ray);
    
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5);
    EXPECT_NEAR(result.normal[0], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[2], 1.0f, 1e-5);
    EXPECT_NEAR(result.u, 0.5f, 1e-5);
    EXPECT_NEAR(result.v, 0.5f, 1e-5);
}

TEST(GeometryTest, TriangleNoIntersection3D) {
    Vector3f v1{0, 0, 0}, v2{1, 0, 0}, v3{0, 1, 0};
    Triangle3f triangle(v1, v2, v3, true);

    Ray3f ray(Vector3f{2, 2, 0}, Vector3f{0, 0, 1}); // Ray parallel to the triangle's plane

    Intersection<float, 3> result = triangle.intersect(ray);
    
    EXPECT_FALSE(result.hit);
}

// Test cases to compare Triangle and TriangleWithNormals intersection results
TEST(GeometryTest, CompareTriangleAndTriangleWithNormalsIntersection) {
    Vector3f v1{0, 0, 0}, v2{1, 0, 0}, v3{0, 1, 0};
    Vector3f n1{0, 0, 1}, n2{0, 0, 1}, n3{0, 0, 1}; // Flat surface for simplicity
    
    Triangle3f triangle(v1, v2, v3, true);
    TriangleWithNormals3f triangleWithNormals(v1, v2, v3, n1, n2, n3, true);

    Ray3f ray(Vector3f{0.5, 0.5, -1}, Vector3f{0, 0, 1}); // Ray towards the center

    Intersection<float, 3> result1 = triangle.intersect(ray);
    Intersection<float, 3> result2 = triangleWithNormals.intersect(ray);

    EXPECT_TRUE(result1.hit);
    EXPECT_TRUE(result2.hit);
    EXPECT_NEAR(result1.t, result2.t, 1e-5);
    
    // For flat triangles, normals should be identical
    EXPECT_NEAR(result1.normal[0], result2.normal[0], 1e-5);
    EXPECT_NEAR(result1.normal[1], result2.normal[1], 1e-5);
    EXPECT_NEAR(result1.normal[2], result2.normal[2], 1e-5);

    EXPECT_NEAR(result1.u, result2.u, 1e-5);
    EXPECT_NEAR(result1.v, result2.v, 1e-5);
}

TEST(GeometryTest, CompareTriangleAndTriangleWithNormalsNoIntersection) {
    Vector3f v1{0, 0, 0}, v2{1, 0, 0}, v3{0, 1, 0};
    Vector3f n1{0, 0, 1}, n2{0, 0, 1}, n3{0, 0, 1};
    
    Triangle3f triangle(v1, v2, v3, true);
    TriangleWithNormals3f triangleWithNormals(v1, v2, v3, n1, n2, n3, true);

    Ray3f ray(Vector3f{2, 2, 0}, Vector3f{0, 0, 1}); // Ray parallel to the triangle's plane

    Intersection<float, 3> result1 = triangle.intersect(ray);
    Intersection<float, 3> result2 = triangleWithNormals.intersect(ray);
    
    EXPECT_FALSE(result1.hit);
    EXPECT_FALSE(result2.hit);
}

TEST(GeometryTest, AABBIntersection3DWithNormal) {
    AABB3f box(Vector3f{-1, -1, -1}, Vector3f{1, 1, 1});
    
    // Ray hits the front face (Z+)
    Ray3f rayFront(Vector3f{0, 0, -2}, Vector3f{0, 0, 1});
    auto result = box.intersects(rayFront);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5); // Distance from -2 to -1
    EXPECT_NEAR(result.normal[0], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[2], 1.0f, 1e-5); // Normal should point +Z

    // Ray hits the back face (Z-)
    Ray3f rayBack(Vector3f{0, 0, 2}, Vector3f{0, 0, -1});
    result = box.intersects(rayBack);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5); // Distance from 2 to 1
    EXPECT_NEAR(result.normal[0], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[2], -1.0f, 1e-5); // Normal should point -Z

    // Ray hits the right face (X+)
    Ray3f rayRight(Vector3f{-2, 0, 0}, Vector3f{1, 0, 0});
    result = box.intersects(rayRight);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5); // Distance from -2 to -1
    EXPECT_NEAR(result.normal[0], 1.0f, 1e-5); // Normal should point +X
    EXPECT_NEAR(result.normal[1], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[2], 0.0f, 1e-5);

    // Ray hits the left face (X-)
    Ray3f rayLeft(Vector3f{2, 0, 0}, Vector3f{-1, 0, 0});
    result = box.intersects(rayLeft);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5); // Distance from 2 to 1
    EXPECT_NEAR(result.normal[0], -1.0f, 1e-5); // Normal should point -X
    EXPECT_NEAR(result.normal[1], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[2], 0.0f, 1e-5);

    // Ray hits the top face (Y+)
    Ray3f rayTop(Vector3f{0, -2, 0}, Vector3f{0, 1, 0});
    result = box.intersects(rayTop);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5); // Distance from -2 to -1
    EXPECT_NEAR(result.normal[0], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], 1.0f, 1e-5); // Normal should point +Y
    EXPECT_NEAR(result.normal[2], 0.0f, 1e-5);

    // Ray hits the bottom face (Y-)
    Ray3f rayBottom(Vector3f{0, 2, 0}, Vector3f{0, -1, 0});
    result = box.intersects(rayBottom);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5); // Distance from 2 to 1
    EXPECT_NEAR(result.normal[0], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], -1.0f, 1e-5); // Normal should point -Y
    EXPECT_NEAR(result.normal[2], 0.0f, 1e-5);

    // Ray not intersecting
    Ray3f rayMiss(Vector3f{2, 2, 2}, Vector3f{0, 0, 1});
    result = box.intersects(rayMiss);
    EXPECT_FALSE(result.hit);
    EXPECT_NEAR(result.t, 0.0f, 1e-5); // No hit, so t should be 0 or remain unchanged
}

TEST(GeometryTest, AABBIntersection2DWithNormal) {
    AABB2f box(Vector2f{-1, -1}, Vector2f{1, 1});
    
    // Ray hits the right face (X+)
    Ray2f rayRight(Vector2f{-2, 0}, Vector2f{1, 0});
    auto result = box.intersects(rayRight);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5);
    EXPECT_NEAR(result.normal[0], 1.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], 0.0f, 1e-5);

    // Ray hits the left face (X-)
    Ray2f rayLeft(Vector2f{2, 0}, Vector2f{-1, 0});
    result = box.intersects(rayLeft);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5);
    EXPECT_NEAR(result.normal[0], -1.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], 0.0f, 1e-5);

    // Ray hits the top face (Y+)
    Ray2f rayTop(Vector2f{0, -2}, Vector2f{0, 1});
    result = box.intersects(rayTop);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5);
    EXPECT_NEAR(result.normal[0], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], 1.0f, 1e-5);

    // Ray hits the bottom face (Y-)
    Ray2f rayBottom(Vector2f{0, 2}, Vector2f{0, -1});
    result = box.intersects(rayBottom);
    EXPECT_TRUE(result.hit);
    EXPECT_NEAR(result.t, 1.0f, 1e-5);
    EXPECT_NEAR(result.normal[0], 0.0f, 1e-5);
    EXPECT_NEAR(result.normal[1], -1.0f, 1e-5);

    // Ray not intersecting
    Ray2f rayMiss(Vector2f{2, 2}, Vector2f{0, 1});
    result = box.intersects(rayMiss);
    EXPECT_FALSE(result.hit);
    EXPECT_NEAR(result.t, 0.0f, 1e-5);
}

TEST(GeometryTest, SphereCentroid) {
    Sphere3f sphere(Vector3f{1, 2, 3}, 1.0f);
    Vector3f centroid = sphere.getCentroid();
    EXPECT_FLOAT_EQ(1.0f, centroid[0]);
    EXPECT_FLOAT_EQ(2.0f, centroid[1]);
    EXPECT_FLOAT_EQ(3.0f, centroid[2]);
}

TEST(GeometryTest, TriangleCentroid) {
    Triangle3f triangle(Vector3f{0, 0, 0}, Vector3f{2, 0, 0}, Vector3f{1, 2, 0});
    Vector3f centroid = triangle.getCentroid();
    EXPECT_NEAR(1.0f, centroid[0], 1e-6f);
    EXPECT_NEAR(0.666667f, centroid[1], 1e-6f);
    EXPECT_NEAR(0.0f, centroid[2], 1e-6f);
}

TEST(GeometryTest, TriangleWithNormalsCentroid) {
    TriangleWithNormals3f triangleWithNormals(
        Vector3f{0, 0, 0}, Vector3f{2, 0, 0}, Vector3f{1, 2, 0},
        Vector3f{0, 0, 1}, Vector3f{0, 0, 1}, Vector3f{0, 0, 1}
    );
    Vector3f centroid = triangleWithNormals.getCentroid();
    EXPECT_NEAR(1.0f, centroid[0], 1e-6f);
    EXPECT_NEAR(0.666667f, centroid[1], 1e-6f);
    EXPECT_NEAR(0.0f, centroid[2], 1e-6f);
}

TEST(GeometryTest, RaySphereIntersection) {
    // Sphere at origin with radius 1
    Sphere3f sphere(Vector3f{0, 0, 0}, 1.0f);

    // Ray that should intersect
    Ray3f intersecting_ray(Vector3f{-2, 0, 0}, Vector3f{1, 0, 0}.normalized());
    auto intersection = sphere.intersect(intersecting_ray);
    
    EXPECT_TRUE(intersection.hit);
    EXPECT_NEAR(intersection.t, 1.0f, 1e-5f);  // Distance from -2 to -1 (surface of sphere)

    // Ray that should not intersect
    Ray3f non_intersecting_ray(Vector3f{2, 0, 0}, Vector3f{1, 0, 0}.normalized()); // Outside the sphere's influence
    intersection = sphere.intersect(non_intersecting_ray);
    
    EXPECT_FALSE(intersection.hit);
}

TEST(GeometryTest, RaySphereNoIntersection) {
    // Sphere at origin with radius 1
    Sphere3f sphere(Vector3f{0, 0, 0}, 1.0f);

    // Rays that should not intersect
    Ray3f non_intersecting_ray(Vector3f{2, 0, 0}, Vector3f{1, 0, 0}.normalized()); // Outside the sphere's influence
    auto intersection = sphere.intersect(non_intersecting_ray);
    
    EXPECT_FALSE(intersection.hit);	
}

TEST(GeometryTest, CreatePrismNormals) {
    std::vector<Triangle3f> box;
	
	box = geom::create_prisma(Vector3f{0.0, 0.0, 0.0}, Vector3f{1, 0, 0}, Vector3f{0, 0, -1}, Vector3f{0, 1, 0});
    auto intersection = box[0].intersect( Ray3f{ Vector3f{0.75, 0.25, 5}, Vector3f{0, 0, -1}.normalized()} );
	EXPECT_TRUE(intersection.hit);
	EXPECT_NEAR(intersection.normal[0],  0.0f, 1e-5);
    EXPECT_NEAR(intersection.normal[1],  0.0f, 1e-5); 
    EXPECT_NEAR(intersection.normal[2],  1.0f, 1e-5);
	box = geom::create_prisma(Vector3f{0.0, 0.0, 0.0}, Vector3f{1, 0, 0}, Vector3f{0, 0, -1}, Vector3f{0, 1, 0});
    intersection = box[1].intersect( Ray3f{ Vector3f{0.25, 0.75, 5}, Vector3f{0, 0, -1}.normalized()} );
	EXPECT_TRUE(intersection.hit);
	EXPECT_NEAR(intersection.normal[0],  0.0f, 1e-5);
    EXPECT_NEAR(intersection.normal[1],  0.0f, 1e-5); 
    EXPECT_NEAR(intersection.normal[2],  1.0f, 1e-5);
}