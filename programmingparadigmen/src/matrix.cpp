#include "matrix.h"

namespace math {

// Default constructor initializes with identity if square, otherwise zeroes
template<typename T, size_t M, size_t N>
Matrix<T, M, N>::Matrix() : data(M, Vector<T, N>(static_cast<T>(0))) {
    if (M == N) {
        for (size_t i = 0; i < M; ++i) {
            data[i][i] = static_cast<T>(1);
        }
    }
}

// Constructor with 2D initializer list
template<typename T, size_t M, size_t N>
Matrix<T, M, N>::Matrix(const std::initializer_list<std::initializer_list<T>>& list) {
    size_t i = 0;
    for (const auto& row : list) {
        assert(i < M && row.size() == N); // Ensure correct dimensions
        data[i++] = Vector<T, N>(row);
    }
}

// Constructor with column vectors
template<typename T, size_t M, size_t N>
Matrix<T, M, N>::Matrix(const Vector<T, M>* columns) {
    for (size_t i = 0; i < M; ++i) {
        data.emplace_back();
        for (size_t j = 0; j < N; ++j) {
            data[i][j] = columns[j][i];
        }
    }
}
// Constructor with row vectors
/*
template<typename T, size_t M, size_t N>
Matrix<T, M, N>::Matrix(const Vector<T, N>* rows) {
    for (size_t i = 0; i < M; ++i) {
        data.push_back(rows[i]);
    }
}*/

// Row access
template<typename T, size_t M, size_t N>
Vector<T, N>& Matrix<T, M, N>::operator[](size_t i) {
    assert(i < M);
    return data[i];
}

template<typename T, size_t M, size_t N>
const Vector<T, N>& Matrix<T, M, N>::operator[](size_t i) const {
    assert(i < M);
    return data[i];
}

// Element access
template<typename T, size_t M, size_t N>
T& Matrix<T, M, N>::operator()(size_t i, size_t j) {
    assert(i < M && j < N);
    return data[i][j];
}

template<typename T, size_t M, size_t N>
const T& Matrix<T, M, N>::operator()(size_t i, size_t j) const {
    assert(i < M && j < N);
    return data[i][j];
}

// Matrix addition
template<typename T, size_t M, size_t N>
Matrix<T, M, N> Matrix<T, M, N>::operator+(const Matrix& other) const {
    Matrix<T, M, N> result;
    for (size_t i = 0; i < M; ++i) {
        result[i] = data[i] + other[i];
    }
    return result;
}

// Matrix subtraction
template<typename T, size_t M, size_t N>
Matrix<T, M, N> Matrix<T, M, N>::operator-(const Matrix& other) const {
    Matrix<T, M, N> result;
    for (size_t i = 0; i < M; ++i) {
        result[i] = data[i] - other[i];
    }
    return result;
}

// Matrix multiplication
template<typename T, size_t M, size_t N>
template<size_t P>
Matrix<T, M, P> Matrix<T, M, N>::operator*(const Matrix<T, N, P>& other) const {
    Matrix<T, M, P> result;
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < P; ++j) {
            T sum = static_cast<T>(0);
            for (size_t k = 0; k < N; ++k) {
                sum += data[i][k] * other[k][j];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

// Scalar multiplication
template<typename T, size_t M, size_t N>
Matrix<T, M, N> Matrix<T, M, N>::operator*(T scalar) const {
    Matrix<T, M, N> result;
    for (size_t i = 0; i < M; ++i) {
        result[i] = data[i] * scalar;
    }
    return result;
}

// Vector multiplication
template<typename T, size_t M, size_t N>
Vector<T, M> Matrix<T, M, N>::operator*(const Vector<T, N>& vector) const {
    Vector<T, M> result;
    for (size_t i = 0; i < M; ++i) {
        result[i] = data[i].dot(vector);
    }
    return result;
}

// Output stream operator
template<typename T, size_t M, size_t N>
std::ostream& operator<<(std::ostream& os, const Matrix<T, M, N>& mat) {
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            os << mat[i][j];
            if (j < N - 1) os << " "; // Space between elements
        }
        if (i < M - 1) os << "\n"; // Newline after each row except the last
    }
    return os;
}

// Explicit instantiations for common types to reduce compile-time overhead
template class math::Matrix<float, 2, 2>;
template class math::Matrix<float, 2, 3>;
template class math::Matrix<float, 3, 2>;
template math::Matrix<float, 2, 2> math::Matrix<float, 2, 2>::operator*<2>(const math::Matrix<float, 2, 2>&) const;
}