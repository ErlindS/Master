#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <vector>
#include <cmath>
#include "camera.h"
#include "shading_algorithms.h"
#include "optical_geometry.h"
#include "geometry.h"
#include "vector.h"
#include "ppm_reader.h"
#include "spatial_structure.h"
#include <tuple>
using namespace math;
using namespace geom;
/**
 * @brief A template class for a ray tracer.
 * 
 * This class encapsulates the logic for ray tracing in a 3D scene, handling 
 * both the rendering process and ray casting. It's templated to allow for 
 * different floating-point precision and different geometric objects.
 *
 * @tparam T The scalar type used for calculations, typically float or double.
 * @tparam ObjectType The type of geometric primitive used in the scene, 
 *                    must support methods like intersect() and getMaterial().
 */
template<typename T, typename ObjectType>
class Raytracer {

	/**
     * @brief A spatial structure for accelerating ray intersection tests.
     */
    std::shared_ptr<SpatialStructure<T, 3, ObjectType>> spatialStructure;

    const int NUM_SAMPLES;
	const int MAX_DEPTH;
protected:	

    std::vector<std::shared_ptr<ObjectType>> shadow_buffer;
	
	void processPixel(int x, int y, const Camera<T>& camera, const std::vector<Light<T, 3>>& lights, Image& image, float gamma);
public:

    /**
     * @brief The color used for background when rays do not intersect with any geometry.
     */
    static const Color<T> BACKGROUND_COLOR;

    /**
     * @brief gamma correction value for color adjustment to match human visual system.
     * 
     * This value is commonly set to 1/2.2 for sRGB color space.
     */
    T gamma = T(1.0) / T(2.3);

    /**
     * @brief Constructor that initializes the spatial structure.
     */
    explicit Raytracer(std::shared_ptr<SpatialStructure<T, 3, ObjectType>> structure, const int NUM_SAMPLES = 1, const int MAX_DEPTH = 5)
        : spatialStructure(structure), NUM_SAMPLES(NUM_SAMPLES), MAX_DEPTH(MAX_DEPTH) { shadow_buffer.resize(MAX_DEPTH, nullptr); }
	
    /**
     * @brief Choose different lighting components for the shading process.
     * 
     * This method returns a tuple indicating which lighting components (ambient, diffuse, reflection, transmission) 
     * to use for the current rendering.
     * 
     * @param material The material properties of an object.
	 *
     * @return std::tuple<bool, bool, bool, bool> Tuple representing the selection of ambient, diffuse, reflection, and transmission lighting.
     */
    virtual std::tuple<bool, bool, bool, bool> choose_lightning(const Material<T, 3>& material) {
		return std::make_tuple(true, true, material.reflectivity > 0.0f, material.transmission > 0.0f);
	}
    
    /**
     * @brief Computes ambient lighting at the intersection point.
     * 
     * @param hitPoint The point of intersection.
     * @param intersection The object intersection details.
     * @param ray The ray being traced.
     * @param material The material properties of the intersected object.
     * @return Color<T> The computed ambient lighting color.
     */
    virtual Color<T> ambientLighting(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const Material<T, 3>& material);

    /**
     * @brief Computes diffuse lighting at the intersection point.
     * 
     * @param hitPoint The point of intersection.
     * @param intersection The object intersection details.
     * @param ray The ray being traced.
     * @param lights A vector of light sources in the scene.
     * @param material The material properties of the intersected object.
     * @return Color<T> The computed diffuse lighting color.
     */
    virtual Color<T> diffuseLighting(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, const Material<T, 3>& material, const int depth, T current_density );

    /**
     * @brief Computes reflection at the intersection point.
     * 
     * @param hitPoint The point of intersection.
     * @param intersection The object intersection details.
     * @param ray The ray being traced.
     * @param lights A vector of light sources in the scene.
     * @param depth The current recursion depth for ray tracing.
     * @param current_density The current optical density of the material.
     * @param material The material properties of the intersected object.
     * @return Color<T> The computed reflection color.
     */
    virtual Color<T> reflection(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, int depth, T current_density, const Material<T, 3>& material);

    /**
     * @brief Computes transmission (refraction) at the intersection point.
     * 
     * @param hitPoint The point of intersection.
     * @param intersection The object intersection details.
     * @param ray The ray being traced.
     * @param lights A vector of light sources in the scene.
     * @param depth The current recursion depth for ray tracing.
     * @param current_density The current optical density of the material.
     * @param material The material properties of the intersected object.
     * @param n1 The refractive index of the medium the ray is exiting.
     * @param n2 The refractive index of the medium the ray is entering.
     * @return Color<T> The computed transmission (refraction) color.
     */
    virtual Color<T> transmission(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, int depth, T current_density, const Material<T, 3>& material, T n1, T n2);

	/**
     * @brief Checks via a shadow ray whether the point is in shadows or not.
     * 
	 * @param hitPoint the point on the surface to be illumnated.
     * @param lights Light sources that contribute to scene illumination.
	 * @param lightDir the normalized direction to the light (output param)
	 * @param depth number of ray bounces (or recursion depth)
	 */
	bool isInShadow(const Vector<T, 3>& hitPoint, const Light<T, 3>& light, Vector<T, 3>& lightDir, int depth)	;
	
    /**
     * @brief Casts a ray through the scene to compute the color at the intersection point.
     * 
     * This method implements the Whitted ray tracing algorithm, which includes 
     * direct illumination, reflections, and refractions. 
     * 
     * @param ray The ray to be cast into the scene.
     * @param objects A vector of geometric objects in the scene to check for intersections.
     * @param lights Light sources that contribute to scene illumination.
     * @param depth The current recursion depth for ray tracing.
     * @param current_density The current optical density of the material.
     * @return Color<T> The computed color at the ray's intersection point or the background color if no intersection.
     */
    Color<T> castRay(const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, int depth, T current_density);

    /**
     * @brief Renders the scene by generating rays for each pixel of the image.
     * 
     * This function uses the camera to generate rays for each pixel, then computes 
     * the color through ray tracing, and applies gamma correction before storing 
     * the result in the image buffer.
     * 
     * @param camera The camera object which defines the perspective and generates rays.
     * @param objects Vector of scene objects to render.
     * @param lights Vector of light sources for shading calculations.
     * @param image The image buffer where the rendered scene will be stored.
     */
    void render(const Camera<T>& camera, const std::vector<Light<T, 3>>& lights, Image& image);
};
/**
 * @brief Definition of the background color for the ray tracer.
 *
 * This color is used when a ray does not hit any object in the scene, 
 * representing the sky or background color.
 */
template<typename T, typename ObjectType>
const Color<T> Raytracer<T, ObjectType>::BACKGROUND_COLOR(0.0f, 0.0f, 0.0f);

#endif // RAYTRACER_H