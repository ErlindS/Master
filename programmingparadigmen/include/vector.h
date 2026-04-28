#ifndef VECTOR_H
#define VECTOR_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <array>


namespace math {
	
/**
 * @brief A template class representing an N-dimensional mathematical vector.
 * 
 * This class provides a versatile representation of vectors in N-dimensional space,
 * supporting fundamental operations such as addition, subtraction, scalar multiplication,
 * dot product, normalization, and geometric computations like projection, orthogonality checks,
 * and cross product (limited to 3D vectors).
 * 
 * @tparam T The scalar type of the vector components (e.g., float, double).
 * @tparam Dim The positive integer representing the dimension of the vector space.
 */
template<typename T, size_t Dim>
class Vector 
{
   std::array<T, Dim> data; ///< Stores the components of the vector.

public:

    /**
     * @brief Default constructor.
     * 
     * Initializes the vector with uninitialized values.
     */
    Vector() noexcept;
    
    /**
     * @brief Constructs a vector where all components are initialized to the same scalar value.
     * 
     * @param value The scalar value used to initialize all components.
     */
	explicit Vector(T value) noexcept;
	
    /**
     * @brief Constructs a vector using an initializer list.
     * 
     * @param list A list of values to initialize the vector.
     */ 
    Vector(const std::initializer_list<T>& list);

    /**
     * @brief Accesses a specific component of the vector.
     * 
     * @param i The index of the component to access.
     * @return Reference to the i-th component.
     */
    T& operator[](size_t i);
    
    /**
     * @brief Accesses a specific component of the vector (const version).
     * 
     * @param i The index of the component to access.
     * @return Const reference to the i-th component.
     */
    const T& operator[](size_t i) const;

    /**
     * @brief Adds another vector to this vector.
     * 
     * Mathematically, if \( \mathbf{a} = (a_1, a_2, ..., a_n) \) and
     * \( \mathbf{b} = (b_1, b_2, ..., b_n) \), then the sum is
     * \( \mathbf{a} + \mathbf{b} = (a_1 + b_1, a_2 + b_2, ..., a_n + b_n) \).
     * 
     * @param other The vector to be added.
     * @return A new vector that is the sum of this vector and @p other.
     */Vector operator+(const Vector& other) const;

    /**
     * @brief Performs in-place vector addition.
     * 
     * @param other The vector to be added.
     * @return Reference to this vector after addition.
     */
    Vector& operator+=(const Vector& other);
    
   /**
     * @brief Subtracts another vector from this vector.
     * 
     * Mathematically, if \( \mathbf{a} \) and \( \mathbf{b} \) are vectors,
     * their difference is \( \mathbf{a} - \mathbf{b} = (a_1 - b_1, a_2 - b_2, ..., a_n - b_n) \).
     * 
     * @param other The vector to be subtracted.
     * @return A new vector that is the difference of this vector and @p other.
     */
    Vector operator-(const Vector& other) const;

    /**
     * @brief Performs in-place vector subtraction.
     * 
     * @param other The vector to be subtracted.
     * @return Reference to this vector after subtraction.
     */
    Vector& operator-=(const Vector& other);
    
    /**
     * @brief Multiplies this vector by a scalar.
     * 
     * Mathematically, if \( \mathbf{a} = (a_1, a_2, ..., a_n) \) and \( c \) is a scalar,
     * then \( c \mathbf{a} = (c a_1, c a_2, ..., c a_n) \).
     * 
     * @param scalar The scalar multiplier.
     * @return A new vector where each component is multiplied by @p scalar.
     */
    Vector operator*(T scalar) const;

    /**
     * @brief Performs component-wise multiplication with another vector.
     * 
     * @param other The vector to multiply by.
     * @return A new vector resulting from element-wise multiplication.
     */
    Vector operator*(const Vector& other) const;

    /**
     * @brief Divides this vector by a scalar.
     * 
     * @param scalar The scalar divisor.
     * @return A new vector where each component is divided by @p scalar.
     */
    Vector operator/(T scalar) const;

    Vector operator-() const; 

	
    /**
     * @brief Compares two vectors for equality.
     */
    bool operator==(const Vector<T, Dim>& rhs) const;

    /**
     * @brief Compares two vectors for inequality.
     */
    bool operator!=(const Vector<T, Dim>& rhs) const;

    /**
     * @brief Computes the dot product with another vector.
     * 
     * Mathematically, the dot product of two vectors \( \mathbf{a} \) and \( \mathbf{b} \) is:
     * \( \mathbf{a} \cdot \mathbf{b} = a_1 b_1 + a_2 b_2 + ... + a_n b_n \).
     * 
     * @param other The vector to compute the dot product with.
     * @return The resulting scalar dot product.
     */
    T dot(const Vector& other) const;

    /**
     * @brief Computes the Euclidean norm (magnitude) of the vector.
     * 
     * @return The magnitude of the vector.
     */
    T length() const;

    /**
     * @brief Normalizes the vector to unit length.
     * 
     * @return A new vector with unit length but the same direction.
     */
    Vector normalized() const;

    /**
     * @brief Projects this vector onto another vector.
     * 
     * @param other The vector onto which this vector is projected.
     * @return The projection vector.
     */
    Vector project_onto(const Vector& other) const;

    /**
     * @brief Checks if this vector is orthogonal to another vector.
     * 
     * @param other The vector to check orthogonality against.
     * @return True if orthogonal, false otherwise.
     */
    bool is_orthogonal_to(const Vector& other) const;

    /**
     * @brief Checks if two vectors are orthonormal.
     * 
     * @param v1 The first vector.
     * @param v2 The second vector.
     * @return True if both vectors are orthonormal, false otherwise.
     */
    static bool are_orthonormal(const Vector& v1, const Vector& v2);

    /**
     * @brief Computes the cross product (only valid for 3D vectors).
     * 
     * Mathematically, the cross product of two 3D vectors \( \mathbf{a} \) and \( \mathbf{b} \) is:
     * \(
     * \mathbf{a} \times \mathbf{b} = (a_2 b_3 - a_3 b_2, a_3 b_1 - a_1 b_3, a_1 b_2 - a_2 b_1)
     * \)
     * 
     * @param other The other vector.
     * @return A new vector that is the cross product result.
     */
    Vector cross(const Vector& other) const;

    /**
     * @brief Computes the scalar triple product of three 3D vectors.
     * 
     * Mathematically, the scalar triple product of vectors \( \mathbf{a}, \mathbf{b}, \mathbf{c} \) is:
     * \(
     * \mathbf{a} \cdot (\mathbf{b} \times \mathbf{c})
     * \)
     * This results in a scalar value that represents the signed volume of the parallelepiped formed by the vectors.
     * 
     * @param a The first vector.
     * @param b The second vector.
     * @param c The third vector.
     * @return The scalar triple product result.
     */
    static T triple_product(const Vector& a, const Vector& b, const Vector& c);

    /**
     * @brief Outputs the vector in a human-readable format.
     */
    template<typename U, size_t D>
    friend std::ostream& operator<<(std::ostream& os, const Vector<U, D>& vec);
    
    /**
     * @brief Scalar multiplication from the left (commutative convenience).
     */
    template<typename U, size_t D>
    friend Vector<U, D> operator*(U scalar, const Vector<U, D>& vec);
};

// Typedefs for common vector sizes
typedef Vector<float, 2> Vector2f;
typedef Vector<float, 3> Vector3f;
typedef Vector<float, 4> Vector4f;

} // end namespace
#endif // VECTOR_H
