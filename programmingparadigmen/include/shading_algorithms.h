#ifndef SHADING_ALGORITHMS_H
#define SHADING_ALGORITHMS_H

#include "geometry.h"
#include <vector>

using namespace math;
using namespace geom;

/**
 * @brief Represents a RGB color.
 *
 * This class encapsulates RGB color values with a range from 0.0 to 1.0 for each component. 
 * It inherits from a generic Vector class to leverage vector operations for color calculations.
 * 
 * @tparam T The type used for color components, should support arithmetic operations.
 */
template<typename T>
class Color : public Vector<T, 3> {
public:
    /**
     * @brief Constructs a Color from individual RGB components.
     * 
     * @param red The red component of the color, in the range [0, 1].
     * @param green The green component of the color, in the range [0, 1].
     * @param blue The blue component of the color, in the range [0, 1].
     */
    Color(T red, T green, T blue) : Color(Vector<T, 3>{red, green, blue}) {}

    /**
     * @brief Constructs a Color from a Vector with three components.
     * 
     * @param vec A Vector representing the RGB color where vec[0] is red, vec[1] is green, and vec[2] is blue.
     */
    Color(const Vector<T, 3>& vec) : Vector<T, 3>(vec) {}

    /**
     * @brief Get the red component of the color.
     * 
     * @return T The red component value.
     */
    T getRed() const;

    /**
     * @brief Get the green component of the color.
     * 
     * @return T The green component value.
     */
    T getGreen() const;

    /**
     * @brief Get the blue component of the color.
     * 
     * @return T The blue component value.
     */
    T getBlue() const;
};

/**
 * @brief Represents material properties for shading and optical behaviors.
 *
 * This class encapsulates all properties that define the optical behavior of a surface.
 *
 * Apply Gamma correction as a post-process, because, color values
 * are not clamped to the range [0, 1] during shading calculations to preserve
 * the full dynamic range.
 */
template<typename T, size_t Dim>
class Material : public BaseMaterial {
public:
    Color<T> color; ///< Color of the surface
    T ka; ///< Ambient reflection coefficient
    T kd; ///< Diffuse reflection coefficient
    T ks; ///< Specular reflection coefficient
    T shininess; ///< Shininess factor for specular highlights
    T reflectivity; ///< Reflectivity of the material
    T transmission; ///< Transmission coefficient (how much light passes through)
    T opticalDensity; ///< Optical density for refraction calculations
	T emission; ///< Amount of light of given color that is emitted
	
    /**
     * @brief Constructor for Material.
     * 
     * @param amb Ambient reflection.
     * @param diff Diffuse reflection.
     * @param spec Specular reflection.
     * @param shine Shininess for specular highlights.
     * @param ref Reflectivity.
     * @param trans Transmission.
     * @param density Optical density.
     */
    Material(Color<T> color, T amb, T diff, T spec, T shine = 0, T ref = 0, T trans = 0, T density = 1, T emission = 0) 
        : BaseMaterial(), color(color), ka(amb), kd(diff), ks(spec), shininess(shine), 
          reflectivity(ref), transmission(trans), opticalDensity(density), emission(emission) {
			  assert(density > 0);
			  }
};

/**
 * @brief Represents a light source in the scene.
 *
 * Defines the position, intensity, and color of light sources for shading calculations.
 */
template<typename T, size_t Dim>
class Light {
public:
    Vector<T, Dim> position; ///< Position of the light source
    Vector<T, Dim> intensity; ///< Intensity of the light source
    Color<T> color; ///< Color of the light source

    /**
     * @brief Constructor for Light.
     * 
     * @param pos Position of the light.
     * @param inten Intensity of the light.
     * @param col Color of the light.
     */
    Light(const Vector<T, Dim>& pos, const Vector<T, Dim>& inten, const Color<T>& col) 
        : position(pos), intensity(inten), color(col) {}
};

/**
 * @brief Defines shading algorithms for ray tracing purposes.
 * 
 * This class contains methods for computing lighting effects like Phong shading,
 * and Lambertian shading.
 */
template<typename T, size_t Dim>
class ShadingAlgorithms {
public:
    /**
     * @brief Constructor initializes the random number generator.
     */
    ShadingAlgorithms()  {}

    /**
     * @brief Computes Lambertian shading for a given intersection.
     * 
     * Lambertian shading assumes that the intensity of reflected light 
     * is the same in all directions (diffuse reflection). 
     * It does not add the ambient color to the result.
	 *
     * @param point The intersection point.
     * @param intersection The intersection details.
     * @param ray The ray that led to the intersection.
     * @param light Light source for shading calculation.
	 * @param lightDir normalized vector that points from the point to the light
     * @param material Material properties of the intersected object.
     * @return Color<T> The computed RGB color after Lambertian shading.
     */
	Color<T> lambertianShading(const Vector<T, Dim>& point, const Intersection<T, Dim>& intersection, const Ray<T, Dim>& ray, const Light<T, Dim>& light, const Vector<T, Dim>& lightDir, const Material<T, Dim>& material) const;

    /**
     * @brief Computes Lambertian shading for a given intersection.
     * 
     * Lambertian shading assumes that the intensity of reflected light 
     * is the same in all directions (diffuse reflection). 
     * It does not add the ambient color to the result.
	 *
     * @param point The intersection point.
     * @param intersection The intersection details.
     * @param ray The ray that led to the intersection.
     * @param light Light source for shading calculation.
     * @param material Material properties of the intersected object.
     * @return Color<T> The computed RGB color after Lambertian shading.
     */
    Color<T> lambertianShading(const Vector<T, Dim>& point, const Intersection<T, Dim>& intersection, const Ray<T, Dim>& ray, const Light<T, Dim>& light, const Material<T, Dim>& material) const;

    /**
     * @brief Computes Lambertian shading for a given intersection and multiple light sources.
     *
     * Lambert's cosine law states that the observed brightness of a Lambertian surface is directly proportional
     * to the cosine of the angle between the observer's line of sight and the surface normal.	 
     * Lambertian shading assumes that the intensity of reflected light 
     * is the same in all directions (diffuse reflection).
     * The ambient color is added to the result.
	 *
     * @param intersection The intersection details.
     * @param ray The ray that led to the intersection.
     * @param lights Vector of light sources for shading calculation.
     * @param material Material properties of the intersected object.
     * @return Color<T> The computed color after Lambertian shading.
     */
    Color<T> lambertianShading(const Intersection<T, Dim>& intersection, const Ray<T, Dim>& ray, const std::vector<Light<T, Dim>>& lights, const Material<T, Dim>& material) const;

    /**
     * @brief Computes Phong shading for a given intersection.
     * 
     * @param intersection The intersection details.
     * @param ray The ray that led to the intersection.
     * @param lights Vector of light sources in the scene.
     * @param viewDir Direction from the intersection point towards the viewer.
     * @param material Material properties of the intersected object.
     * @return Vector<T, Dim> The computed color after shading.
     */
    Color<T> phongShading(const Intersection<T, Dim>& intersection, const Ray<T, Dim>& ray, const std::vector<Light<T, Dim>>& lights, 
                                const Vector<T, Dim>& viewDir, const Material<T, Dim>& material) const;
};


// Typedef for common usage
typedef ShadingAlgorithms<float, 3> ShadingAlgorithms3f;
typedef Material<float, 3> Material3f;
typedef Light<float, 3> Light3f;

#endif // SHADING_ALGORITHMS_H