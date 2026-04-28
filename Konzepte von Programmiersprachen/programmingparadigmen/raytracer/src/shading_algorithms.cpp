// shading_algorithms.cpp

#include "shading_algorithms.h"
#include <cmath>
#include <algorithm>
#include <cassert>
#include <iostream>

template<typename T, size_t Dim>
bool isNormalized(const Vector<T, Dim>& v) {
    T length = v.length();
    return (length > std::numeric_limits<T>::epsilon()) && (length > T(0.99999) && length < T(1.00001));
}


// Color class methods
template<typename T>
T Color<T>::getRed() const { return (*this)[0]; }
template<typename T>
T Color<T>::getGreen() const { return (*this)[1]; }
template<typename T>
T Color<T>::getBlue() const { return (*this)[2]; }

template<typename T, size_t Dim>
Color<T> ShadingAlgorithms<T, Dim>::lambertianShading(const Vector<T, Dim>& point, const Intersection<T, Dim>& intersection, const Ray<T, Dim>& ray, const Light<T, Dim>& light, const Vector<T, Dim>& lightDir, const Material<T, Dim>& material) const {
	assert(isNormalized(intersection.normal));

    T dot = std::max(T{0}, intersection.normal.dot(lightDir));
	
    Color<T> diffuseColor = material.color * material.kd;
    Color<T> lightColor = light.color * dot;
    
    return diffuseColor * lightColor;
}

template<typename T, size_t Dim>
Color<T> ShadingAlgorithms<T, Dim>::lambertianShading(const Vector<T, Dim>& point, const Intersection<T, Dim>& intersection, const Ray<T, Dim>& ray, const Light<T, Dim>& light, const Material<T, Dim>& material) const {
	assert(isNormalized(intersection.normal));
    Vector<T, Dim> lightDir = (light.position - point).normalized();
	
    return lambertianShading(point, intersection, ray, light, lightDir, material);
}

template<typename T, size_t Dim>
Color<T> ShadingAlgorithms<T, Dim>::lambertianShading(const Intersection<T, Dim>& intersection, const Ray<T, Dim>& ray, const std::vector<Light<T, Dim>>& lights, const Material<T, Dim>& material) const {
    Vector<T, Dim> point = ray.get_origin() + intersection.t * ray.get_direction();
    Color<T> result = material.color * material.ka; // Start with ambient light
    for (const auto& light : lights) {
        result += lambertianShading(point, intersection, ray, light, material);
    }
	return result;
}

template<typename T, size_t Dim>
Color<T> ShadingAlgorithms<T, Dim>::phongShading(const Intersection<T, Dim>& intersection, const Ray<T, Dim>& ray, const std::vector<Light<T, Dim>>& lights, 
                                                const Vector<T, Dim>& viewDir, const Material<T, Dim>& material) const {
    Vector<T, Dim> point = ray.get_origin() + intersection.t * ray.get_direction();
    Color<T> color = material.color * material.ka; // Ambient light

    for (const auto& light : lights) {
        Vector<T, Dim> lightDir = (light.position - point).normalized();
		T diffuseDot = intersection.normal.dot(lightDir);
        T diffuse = std::max(T(0.0), diffuseDot);

        // Calculate reflection direction
        Vector<T, Dim> reflectDir = (2 * diffuseDot * intersection.normal - lightDir).normalized();
        T specular = std::pow(std::max(T(0.0), reflectDir.dot(viewDir)), material.shininess);

        Color<T> lightColor(light.color); // Assuming light color is in RGB format
        Color<T> diffuseLight = material.color * material.kd * diffuse * lightColor * Color<T>(light.intensity);
        Color<T> specularLight = lightColor * material.ks * specular * Color<T>(light.intensity);

        color += diffuseLight + specularLight;
    }

    return color;
}
