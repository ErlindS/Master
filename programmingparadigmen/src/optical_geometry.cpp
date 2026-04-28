#include "optical_geometry.h"
#include <cstdlib>
#include <cassert>
#include "optical_geometry.h"
#include <cassert>

template<typename T, size_t Dim>
T OpticalGeometry<T, Dim>::schlickFresnel(T cosTheta, T n1, T n2) {
    assert(cosTheta >= T(0) && cosTheta <= T(1)); // Cosine should be between 0 and 1
    assert(n1 > T(0) && n2 > T(0)); // Refractive indices should be positive

    T r0 = std::pow((n1 - n2) / (n1 + n2), 2);
    return r0 + (1 - r0) * std::pow(1 - cosTheta, 5);
}

template<typename T, size_t Dim>
Vector<T, Dim> OpticalGeometry<T, Dim>::reflect(const Ray<T, Dim>& incident, const Vector<T, Dim>& normal) {
	T cosine = incident.get_direction().dot(normal);
    return incident.get_direction() - 2 * cosine * normal;
}

template<typename T, size_t Dim>
bool OpticalGeometry<T, Dim>::refract(const Ray<T, Dim>& incident, const Vector<T, Dim>& normal, T n1, T n2, Vector<T, Dim>& refractedDirection) {
    assert(n1 > T(0) && n2 > T(0)); // Refractive indices should be positive
	T eta = n1 / n2;
	T cosTheta = std::min(-incident.get_direction().dot(normal), T(1));
	T sinTheta = std::sqrt(1 - cosTheta * cosTheta);
	
	if (eta * sinTheta > 1) {  // Total internal reflection
		return false;
	}

	T k = 1 - eta * eta * (1 - cosTheta * cosTheta);
	refractedDirection = eta * incident.get_direction() + (eta * cosTheta - std::sqrt(k)) * normal;
	return true;
}

template<typename T, size_t Dim>
bool OpticalGeometry<T, Dim>::traceRay(const Ray<T, Dim>& incident, const Vector<T, Dim>& normal, T n1, T n2, Vector<T, Dim>& transmissionDirection, Vector<T, Dim>& reflectionDirection, T randomValue) {
    assert(randomValue >= T(0) && randomValue <= T(1)); // Random value should be in [0, 1]
    assert(n1 > T(0) && n2 > T(0)); // Refractive indices should be positive

	T cosTheta = std::max(T(0), -incident.get_direction().dot(normal));
	T R = schlickFresnel(cosTheta, n1, n2);

	if (randomValue < R) {
        // Partial reflection occurred
        reflectionDirection = reflect(incident, normal);
        transmissionDirection = reflectionDirection; // Set transmission direction to reflection direction
        return true;
    } else {
        // Refraction
        if (refract(incident, normal, n1, n2, transmissionDirection)) {
            return false; // Only refraction, no additional reflection
        } else {
            // Total internal reflection
            reflectionDirection = reflect(incident, normal);
            transmissionDirection = reflectionDirection; // Set transmission direction to reflection direction
            return true; // Partial reflection occurred due to TIR
        }
    }
}
