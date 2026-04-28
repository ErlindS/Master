#include "vector.h"
#include <cmath> 
#include <limits> 
#include <algorithm>
namespace math {

template<typename T, size_t Dim>
Vector<T, Dim>::Vector() noexcept  {  this->data = {}; }
    
template<typename T, size_t Dim>
Vector<T, Dim>::Vector(T value) noexcept  { this->data.fill(value); }
	
template<typename T, size_t Dim>
Vector<T, Dim>::Vector(const std::initializer_list<T>& list) {
	assert(list.size() == Dim && "Initializer list size must match Dim");
	size_t i = 0;
    for (const auto& elem : list) {
        if (i >= Dim) break;
        this->data[i++] = elem;
    }
}
	
template<typename T, size_t Dim>
T& Vector<T, Dim>::operator[](size_t i) {
    assert(i < Dim);
    return data[i];
}

template<typename T, size_t Dim>
const T& Vector<T, Dim>::operator[](size_t i) const {
    assert(i < Dim);
    return data[i];
}

template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::operator+(const Vector& other) const {
    assert(other.Dim == Dim); 
    Vector result(Dim); 
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] + other[i];
    }
    return result;
}

template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::operator-(const Vector& other) const {
    assert(other.Dim == Dim); 
    Vector result(Dim); 
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] - other[i];
    }
    return result;
}


template<typename T, size_t Dim>
Vector<T, Dim> & Vector<T, Dim>::operator+=(const Vector& other) {
    assert(other.Dim == Dim); 
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] += other[i];
    }
    return *this;
}
	
template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::operator*(const Vector& other) const {
    assert(other.Dim == Dim);
    Vector result(Dim);
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] * other[i];
    }
    return result;
}



template<typename T, size_t Dim>
Vector<T, Dim> & Vector<T, Dim>::operator-=(const Vector& other) {
    assert(other.Dim == Dim); 
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] -= other[i];
    }
    return *this;
}

template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::operator*(T scalar) const {
    Vector result;
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] * scalar;
    }
    return result;
}

template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::operator/(T scalar) const {
    assert(scalar != 0);
    Vector result;
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] / scalar;
    }
    return result;
}

template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::operator-() const {
    Vector result;
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = -data[i];
    }
    return result;
}

template<typename T, size_t Dim>
bool Vector<T, Dim>::operator==(const Vector<T, Dim>& rhs) const {
    assert(rhs.Dim == Dim);
    for (size_t i = 0; i < Dim; ++i) {
        if (std::abs(data[i] - rhs.data[i]) > std::numeric_limits<T>::epsilon()) {
            return false;
        }
    }
    return true;
}

template<typename T, size_t Dim>
bool Vector<T, Dim>::operator!=(const Vector<T, Dim>& rhs)  const {
    return !(*this == rhs);
}

template<typename T, size_t Dim>
T Vector<T, Dim>::dot(const Vector& other) const {
    assert(other.Dim == Dim);
    T result{}; 
    for (size_t i = 0; i < Dim; ++i) {
        result += data[i] * other[i];
    }
    return result;
}

template<typename T, size_t Dim>
T Vector<T, Dim>::length() const {
    return std::sqrt(dot(*this));
}

template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::normalized() const {
    T len = length();
    assert(len != 0);
    return (*this) * (1.0 / len); 
}

template<typename U, size_t D>
std::ostream& operator<<(std::ostream& os, const Vector<U, D>& vec) {
    os << "(";
    for (size_t i = 0; i < D; ++i) {
        if (i > 0) os << ", ";
        os << vec[i];
    }
    os << ")";
    return os;
}

template<typename U, size_t D>
Vector<U, D> operator*(U scalar, const Vector<U, D>& vec) {
	return vec * scalar;
}
	
// Cross product (only for 3D vectors)
template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::cross(const Vector & other) const {
    assert(Dim == 3);
    assert(other.Dim == Dim);
	if (Dim == 3) {
		return Vector<T, Dim>{
			data[1] * other[2] - data[2] * other[1],
			data[2] * other[0] - data[0] * other[2],
			data[0] * other[1] - data[1] * other[0]
		};
	}
	throw;
}

// Triple Product (only for 3D vectors)
template<typename T, size_t Dim>
T Vector<T, Dim>::triple_product(const Vector & a, const Vector& b, const Vector& c) {
    assert(Dim == 3);
    return a.dot(b.cross(c));
}

template<typename T, size_t Dim>
Vector<T, Dim> Vector<T, Dim>::project_onto(const Vector& other) const {
    assert(other.length() != 0);
    T scalar = this->dot(other) / other.dot(other);
    return other * scalar;
}

template<typename T, size_t Dim>
bool Vector<T, Dim>::is_orthogonal_to(const Vector& other) const {
    assert(other.Dim == Dim);
    return std::abs(this->dot(other)) < std::numeric_limits<T>::epsilon();
}

template<typename T, size_t Dim>
bool Vector<T, Dim>::are_orthonormal(const Vector& v1, const Vector& v2) {
	assert(v1.Dim == Dim && v2.Dim == Dim);
    return v1.is_orthogonal_to(v2) && std::abs(v1.length() - T{1}) < std::numeric_limits<T>::epsilon() && std::abs(v2.length() - T{1}) < std::numeric_limits<T>::epsilon();
}

} // end namespace