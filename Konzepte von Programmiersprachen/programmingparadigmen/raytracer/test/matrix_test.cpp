#include <gtest/gtest.h>
#include "matrix.h" // Assuming this is your header file's name
#include <iostream>

namespace {

/*
// Test for Matrix constructors
TEST(MatrixTest, Constructors) {
    // Test default constructor for a 2x2 matrix
    math::Matrix<float, 2, 2> m1;
    EXPECT_FLOAT_EQ(m1(0, 0), 1.0f); // Default should be identity for square matrices
    EXPECT_FLOAT_EQ(m1(0, 1), 0.0f);
    EXPECT_FLOAT_EQ(m1(1, 0), 0.0f);
    EXPECT_FLOAT_EQ(m1(1, 1), 1.0f);


	
    // Test constructor with initializer list for a 2x3 matrix
    math::Matrix<float, 2, 3> m2 = {{1, 2, 3}, {4, 5, 6}};
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            EXPECT_FLOAT_EQ(m2(i, j), (i * 3 + j + 1)); // Check each element
        }
    }
	

    // Test constructor with column vectors for a 3x2 matrix
    math::Vector<float, 3> col1{1, 4, 7};
    math::Vector<float, 3> col2{2, 5, 8};
    math::Matrix<float, 3, 2> m3( {col1, col2});
    EXPECT_FLOAT_EQ(m3(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m3(0, 1), 2.0f);
    EXPECT_FLOAT_EQ(m3(1, 0), 4.0f);
    EXPECT_FLOAT_EQ(m3(1, 1), 5.0f);
    EXPECT_FLOAT_EQ(m3(2, 0), 7.0f);
    EXPECT_FLOAT_EQ(m3(2, 1), 8.0f);
    // Test constructor with row vectors for a 2x2 matrix
	std::cout << "A" << std::endl;
    math::Vector<float, 2> row1{1, 2};
    math::Vector<float, 2> row2{3, 4};
    math::Matrix<float, 2, 2> m4(&row1);
	std::cout << "A" << std::endl;
    m4 = math::Matrix<float, 2, 2>(&row2); // Again, for demonstration
    EXPECT_FLOAT_EQ(m4(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m4(0, 1), 2.0f);
    EXPECT_FLOAT_EQ(m4(1, 0), 3.0f);
    EXPECT_FLOAT_EQ(m4(1, 1), 4.0f);
}

// Test for Matrix operations
TEST(MatrixTest, Operations) {
    math::Matrix<float, 2, 2> m1 = {{1, 2}, {3, 4}};
    math::Matrix<float, 2, 2> m2 = {{5, 6}, {7, 8}};

    // Addition
    math::Matrix<float, 2, 2> m_add = m1 + m2;
    EXPECT_FLOAT_EQ(m_add(0, 0), 6.0f);
    EXPECT_FLOAT_EQ(m_add(1, 1), 12.0f);

    // Subtraction
    math::Matrix<float, 2, 2> m_sub = m1 - m2;
    EXPECT_FLOAT_EQ(m_sub(0, 0), -4.0f);
    EXPECT_FLOAT_EQ(m_sub(1, 1), -4.0f);

    // Scalar multiplication
    float scalar = 2.0f;
    math::Matrix<float, 2, 2> m_scalar = m1 * scalar;
    EXPECT_FLOAT_EQ(m_scalar(0, 0), 2.0f);
    EXPECT_FLOAT_EQ(m_scalar(1, 1), 8.0f);

    //Matrix multiplication
    math::Matrix<float, 2, 2> m_mult = m1 * m2;
    EXPECT_FLOAT_EQ(m_mult(0, 0), 19.0f);
    EXPECT_FLOAT_EQ(m_mult(1, 1), 50.0f);
	
    // Vector multiplication (assuming you have a Vector class that supports dot product)
    math::Vector<float, 2> vec{1, 1};
    math::Vector<float, 2> m_vec = m1 * vec;
    EXPECT_FLOAT_EQ(m_vec[0], 3.0f);
    EXPECT_FLOAT_EQ(m_vec[1], 7.0f);
}
*/
} // namespace
