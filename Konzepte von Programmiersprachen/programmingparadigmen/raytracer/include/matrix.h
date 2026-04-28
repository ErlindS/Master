#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"
#include <cassert>
#include <vector>
#include <iostream>

namespace math {

/**
 * @brief A template class representing an MxN matrix.
 * 
 * This class provides basic matrix operations including addition, subtraction, 
 * multiplication, scalar multiplication, and vector multiplication.
 * 
 * @tparam T The scalar type of the matrix elements (e.g., float, double).
 * @tparam M The number of rows in the matrix.
 * @tparam N The number of columns in the matrix.
 */
template<typename T, size_t M, size_t N>
class Matrix {
private:
    std::vector<Vector<T, N>> data; ///< Each row is a Vector of dimension N.

public:
    /**
     * @brief Constructs a matrix with identity values if M == N, otherwise zeroes.
     */
    Matrix();

    /**
     * @brief Constructs a matrix from a 2D initializer list.
     * 
     * @param list A 2D initializer list to fill the matrix.
     */
     Matrix(const std::initializer_list<std::initializer_list<T> >& list);

    /**
     * @brief Constructs a matrix using column vectors.
     * 
     * This constructor takes N vectors each of size M to form the columns of the matrix.
     * This effectively transposes the input vectors to form the matrix data structure.
     * 
     * @param columns An array of N vectors each representing a column of the matrix.
     */
    explicit Matrix(const Vector<T, M>* columns);


    /**
     * @brief Accesses a specific row of the matrix.
     * 
     * @param i The index of the row to access.
     * @return Reference to the i-th row.
     */
    Vector<T, N>& operator[](size_t i);

    /**
     * @brief Accesses a specific row of the matrix (const version).
     * 
     * @param i The index of the row to access.
     * @return Const reference to the i-th row.
     */
    const Vector<T, N>& operator[](size_t i) const;

    /**
     * @brief Accesses a specific element of the matrix.
     * 
     * @param i The index of the row.
     * @param j The index of the column.
     * @return Reference to the element at position (i, j).
     */
    T& operator()(size_t i, size_t j);

    /**
     * @brief Accesses a specific element of the matrix (const version).
     * 
     * @param i The index of the row.
     * @param j The index of the column.
     * @return Const reference to the element at position (i, j).
     */
    const T& operator()(size_t i, size_t j) const;

    /**
     * @brief Adds another matrix to this matrix.
     * 
     * Mathematically, if \( A = [a_{ij}] \) and \( B = [b_{ij}] \), then:
     * \( A + B = [a_{ij} + b_{ij}] \).
     * 
     * @param other The matrix to be added.
     * @return A new matrix that is the sum of this matrix and @p other.
     */
    Matrix operator+(const Matrix& other) const;

    /**
     * @brief Subtracts another matrix from this matrix.
     * 
     * Mathematically, if \( A = [a_{ij}] \) and \( B = [b_{ij}] \), then:
     * \( A - B = [a_{ij} - b_{ij}] \).
     * 
     * @param other The matrix to be subtracted.
     * @return A new matrix that is the difference of this matrix and @p other.
     */
    Matrix operator-(const Matrix& other) const;

    /**
     * @brief Multiplies this matrix with another matrix.
     * 
     * Mathematically, if \( A = [a_{ij}] \) is MxN and \( B = [b_{ij}] \) is NxP, then:
     * \( C = AB \) where \( c_{ij} = \sum_{k=1}^{N} a_{ik} b_{kj} \).
     * 
     * @param other The matrix to multiply with.
     * @return A new matrix resulting from the multiplication.
     */
    template<size_t P>
    Matrix<T, M, P> operator*(const Matrix<T, N, P>& other) const;

    /**
     * @brief Multiplies this matrix by a scalar.
     * 
     * Mathematically, if \( A = [a_{ij}] \) and \( c \) is a scalar, then:
     * \( cA = [c a_{ij}] \).
     * 
     * @param scalar The scalar multiplier.
     * @return A new matrix where each element is multiplied by @p scalar.
     */
    Matrix operator*(T scalar) const;

    /**
     * @brief Multiplies this matrix by a vector.
     * 
     * Mathematically, if \( A = [a_{ij}] \) is MxN and \( \mathbf{v} = (v_1, v_2, ..., v_N) \), then:
     * \( A \mathbf{v} = \sum_{j=1}^{N} a_{ij} v_j \) for each i.
     * 
     * @param vector The vector to multiply by.
     * @return A new vector resulting from the multiplication.
     */
    Vector<T, M> operator*(const Vector<T, N>& vector) const;

    /**
     * @brief Outputs the matrix in a human-readable format.
     * 
     * Each row of the matrix is printed on a new line, with elements separated by spaces.
     * 
     * @param os The output stream to write to.
     * @param mat The matrix to output.
     * @return A reference to the output stream for chaining.
     */
    template<typename U, size_t R, size_t C>
    friend std::ostream& operator<<(std::ostream& os, const Matrix<U, R, C>& mat);
};

typedef Matrix<float, 2, 2> Matrix2f; ///< 2x2 matrix of floats
typedef Matrix<float, 3, 3> Matrix3f; ///< 3x3 matrix of floats
typedef Matrix<float, 4, 4> Matrix4f; ///< 4x4 matrix of floats

} // end namespace

#endif // MATRIX_H
