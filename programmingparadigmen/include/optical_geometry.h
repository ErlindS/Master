#ifndef OPTICAL_GEOMETRY_H
#define OPTICAL_GEOMETRY_H

#include "geometry.h"
#include <cmath>

using namespace math;
using namespace geom;

/**
 * @brief Provides methods for optical ray operations like reflection, refraction, and transmission.
 *
 * This class uses geometric classes to simulate optical interactions based on fundamental physics principles:
 * - Reflection: Based on the law of reflection where the angle of incidence equals the angle of reflection.
 * - Refraction: Based on Snell's law, where light bends at the interface between two media with different refractive indices.
 * - Transmission vs Reflection: Uses a simplified model of the Fresnel equations for deciding between reflection and transmission.
 *
 * @tparam T The scalar type used for calculations.
 * @tparam Dim The dimension of the space (e.g., 3 for 3D space).
 */
template<typename T, size_t Dim>
class OpticalGeometry {
public:

/**
 * @brief Computes the reflection coefficient using Schlick's approximation of the Fresnel equations.
 *
 * Schlick's approximation provides a simpler method to estimate the amount of reflected light at an interface
 * between two materials with different refractive indices. It's particularly useful for real-time rendering 
 * because it avoids the need for solving complex Fresnel equations for each interaction. 
 * 
 * The formula used is:
 * R ≈ R0 + (1 - R0)(1 - cos(θ))^5, where:
 * - R is the reflection coefficient
 * - R0 is the reflection at normal incidence ((n1 - n2)^2 / (n1 + n2)^2)
 * - θ is the angle of incidence (here given as cos(θ))
 *
 * @param cosTheta The cosine of the angle of incidence. Should be in the range [0, 1] for physical accuracy.
 * @param n1 Refractive index of the incident medium.
 * @param n2 Refractive index of the transmission medium.
 * @return T The reflection coefficient calculated using Schlick's approximation.
 */
    static T schlickFresnel(T cosTheta, T n1, T n2);

    /**
     * @brief Calculates the direction of the reflected ray. given an incident ray and surface normal.
     * 
     * Uses the vector form of the law of reflection: 
     * R = I - 2 * (I · N) * N where R is the reflected ray, I is the incident direction, and N is the surface normal.
     *
     * @param incident The incoming ray.
     * @param normal The surface normal at the point of incidence.
     * @return Vector<T, Dim> The direction of the reflected ray.
     */
    static Vector<T, Dim> reflect(const Ray<T, Dim>& incident, const Vector<T, Dim>& normal);

    /**
     * @brief Calculates the refracted ray using Snell's Law.
     * 
     * Snell's Law states: n1 * sin(θ1) = n2 * sin(θ2), where n1 and n2 are refractive indices, 
     * θ1 is the angle of incidence, and θ2 is the angle of refraction. This method computes 
     * the refracted direction using the vector form of this law.
     * 
     * If total internal reflection occurs (sin(θ2) > 1), it returns false.
     *
     * @param incident The incoming ray.
     * @param normal The surface normal at the point of incidence. Pointing away from the surface.
     * @param n1 Refractive index of the medium the ray is coming from.
     * @param n2 Refractive index of the medium the ray is entering.
	 * @param refractedDirection Direction of refracted ray, only set if no total internal reflection occurs.
     * @return true if there is a refraction or false for total internal reflection.
     */
    static bool refract(const Ray<T, Dim>& incident, const Vector<T, Dim>& normal, T n1, T n2, Vector<T, Dim> &refractedDirection);

    /**
     * @brief Applies the appropriate optical operation (reflection and/or refraction) based on material properties.
     * 
     * This method decides whether to reflect or refract the ray based on the material's properties. 
	 * It also includes partial reflection. The calculation is based on Schlick-Fresnel.
     *
     * @param incident The incoming ray.
     * @param normal The surface normal.
     * @param n1 Refractive index of the medium the ray is coming from.
     * @param n2 Refractive index of the medium the ray is entering.
     * @param transmissionDirection The resulting direction of the transmission ray.
     * @param reflectionDirection The optional resulting direction of the partial reflection direction.
     * @param randomValue A value between 0 and 1 to decide whether to reflect (larger values) or not (lower values).
	 * @return true if an additional partial reflection occurs.
     */
    static bool traceRay(const Ray<T, Dim>& incident, const Vector<T, Dim>& normal, T n1, T n2, Vector<T, Dim>& transmissionDirection, Vector<T, Dim>& reflectionDirection, T randomValue);
};

// Typedefs for common types
typedef OpticalGeometry<float, 3> OpticalGeometry3f;

#endif // OPTICAL_GEOMETRY_H