#include <gtest/gtest.h>
#include "vector.h"

using namespace math;

TEST(VectorTest, BasicOperations) {
	Vector3f v0(0.0f);
	EXPECT_FLOAT_EQ(v0[0], 0.0f);
    EXPECT_FLOAT_EQ(v0[1], 0.0f);
    EXPECT_FLOAT_EQ(v0[2], 0.0f);
    
	Vector3f v1{1.0f, 2.0f, 3.0f};
    Vector3f v2{4.0f, 5.0f, 6.0f};

    // Addition
    Vector3f sum = v1 + v2;
    EXPECT_FLOAT_EQ(sum[0], 5.0f);
    EXPECT_FLOAT_EQ(sum[1], 7.0f);
    EXPECT_FLOAT_EQ(sum[2], 9.0f);

    // Subtraction
    Vector3f diff = v1 - v2;
    EXPECT_FLOAT_EQ(diff[0], -3.0f);
    EXPECT_FLOAT_EQ(diff[1], -3.0f);
    EXPECT_FLOAT_EQ(diff[2], -3.0f);

    // Scalar multiplication
    Vector3f scaled = v1 * 2.0f;
    EXPECT_FLOAT_EQ(scaled[0], 2.0f);
    EXPECT_FLOAT_EQ(scaled[1], 4.0f);
    EXPECT_FLOAT_EQ(scaled[2], 6.0f);

    // Scalar multiplication
    scaled = 2.0f * v1;
    EXPECT_FLOAT_EQ(scaled[0], 2.0f);
    EXPECT_FLOAT_EQ(scaled[1], 4.0f);
    EXPECT_FLOAT_EQ(scaled[2], 6.0f);
	
	// Scalar division
    scaled = v1 / 0.5f;
    EXPECT_FLOAT_EQ(scaled[0], 2.0f);
    EXPECT_FLOAT_EQ(scaled[1], 4.0f);
    EXPECT_FLOAT_EQ(scaled[2], 6.0f);

    // Dot product
    EXPECT_FLOAT_EQ(v1.dot(v2), 32.0f);

    // Length
    EXPECT_FLOAT_EQ(v1.length(), std::sqrt(14.0f));

    // Normalization
    Vector3f normalized = v1.normalized();
    EXPECT_FLOAT_EQ(normalized.length(), 1.0f);

    // Cross product
    Vector3f cross = v1.cross(v2);
    EXPECT_FLOAT_EQ(cross[0], -3.0f);
    EXPECT_FLOAT_EQ(cross[1], 6.0f);
    EXPECT_FLOAT_EQ(cross[2], -3.0f);

    // Equality
	EXPECT_EQ(cross, (Vector3f{-3.0f, 6.0f, -3.0f}));
}

TEST(VectorTest, Projection) {
    Vector3f v1{1.0f, 2.0f, 3.0f};
    Vector3f v2{4.0f, 5.0f, 6.0f};
    Vector3f projection = v1.project_onto(v2);
    Vector3f expected{1.6623377f, 2.0779221f, 2.4935064f};
    float delta = 1e-5f;  // Tolerance for float comparison

    EXPECT_NEAR(projection[0], expected[0], delta);
    EXPECT_NEAR(projection[1], expected[1], delta);
    EXPECT_NEAR(projection[2], expected[2], delta);
}

TEST(VectorTest, Orthogonality) {
    Vector3f v1{1.0f, 0.0f, 0.0f};
    Vector3f v2{0.0f, 1.0f, 0.0f};
    EXPECT_TRUE(v1.is_orthogonal_to(v2));
    
    Vector3f v3{1.0f, 1.0f, 0.0f};
    EXPECT_FALSE(v1.is_orthogonal_to(v3));
}

TEST(VectorTest, Orthonormality) {
    Vector3f v1{1.0f, 0.0f, 0.0f};
    Vector3f v2{0.0f, 1.0f, 0.0f};
    EXPECT_TRUE(Vector3f::are_orthonormal(v1, v2));
    
    Vector3f v3{1.0f, 1.0f, 0.0f};
    EXPECT_FALSE(Vector3f::are_orthonormal(v1, v3));
}


TEST(VectorTest, TripleProduct) {
    Vector3f a{1.0f, 2.0f, 3.0f};
    Vector3f b{4.0f, 5.0f, 6.0f};
    Vector3f c{7.0f, 8.0f, 9.0f};
    EXPECT_FLOAT_EQ(Vector3f::triple_product(a, b, c), 0.0f);
}


