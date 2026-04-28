#include "raytracer.h"
#include "statistics.h"
#include <chrono>
#include <iostream>
#include <iomanip>

template<typename T, typename ObjectType>
Color<T> Raytracer<T, ObjectType>::ambientLighting(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const Material<T, 3>& material) {
    return material.color * material.ka; // Ambient lighting
}

template<typename T, typename ObjectType>
Color<T> Raytracer<T, ObjectType>::diffuseLighting(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, const Material<T, 3>& material, const int depth, T current_density) {
    Color<T> resultColor{0, 0, 0};
    ShadingAlgorithms<T, 3> shading;
    for (const auto& light : lights) {
        Vector<T, 3> lightDir;
        if (!isInShadow(hitPoint, light, lightDir, depth)) {
            resultColor = resultColor + shading.lambertianShading(hitPoint, intersection, ray, light, lightDir, material);
        }
    }
    return resultColor;
}

template<typename T, typename ObjectType>
Color<T> Raytracer<T, ObjectType>::reflection(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, int depth, T current_density, const Material<T, 3>& material) {
    if (material.reflectivity > 0.0f) {
        Vector<T, 3> reflectDir = OpticalGeometry<T, 3>::reflect(ray, intersection.normal).normalized();
        Ray<T, 3> reflectRay(hitPoint + intersection.normal * T(0.001), reflectDir);
        Color<T> reflectColor = castRay(reflectRay, lights, depth + 1, current_density);
        return reflectColor * material.reflectivity;
    }
    return Color<T>{0, 0, 0};
}

template<typename T, typename ObjectType>
Color<T> Raytracer<T, ObjectType>::transmission(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, int depth, T current_density, const Material<T, 3>& material, T n1, T n2) {
    if (material.transmission > 0.0f) {
        Vector<T, 3> refractDir;
        if (OpticalGeometry<T, 3>::refract(ray, intersection.normal, n1, n2, refractDir)) {
            Ray<T, 3> refractRay(hitPoint - intersection.normal * T(0.001), refractDir.normalized());
			Color<T> refractColor = castRay(refractRay, lights, depth + 1, n2);
            return refractColor * material.transmission;
        }
    }
    return Color<T>{0, 0, 0};
}
	
template<typename T, typename ObjectType>
bool Raytracer<T, ObjectType>::isInShadow(const Vector<T, 3>& hitPoint, 
                                          const Light<T, 3>& light, Vector<T, 3>& lightDir, int depth)  {
	lightDir = (light.position - hitPoint).normalized();
	static constexpr T SHADOW_BIAS = 1e-3;
    Ray<T, 3> shadowRay(hitPoint + lightDir * SHADOW_BIAS, lightDir);
    ObjectIntersection<T, 3, ObjectType> shadowIntersection;
    T lightDistance = (light.position - hitPoint).length();
    
	if (statistic::stat.shadow_buffer_is_on ) {
		statistic::stat.no_of_shadowrays++;
	    if ( shadow_buffer[depth] != nullptr ) {
		    statistic::stat.shadow_buffer_tests++;
		    shadowIntersection =  ObjectIntersection( shadow_buffer[depth]->intersect(shadowRay), shadow_buffer[depth]);
            if (shadowIntersection.hit && shadowIntersection.t < lightDistance) {
                statistic::stat.shadow_buffer_hits++;
                return true;  
            }
	    }
    }
    shadowIntersection = { Intersection<T, 3> {false, lightDistance, Vector<T, 3>(), 0, 0}, nullptr};
    shadowIntersection = spatialStructure->findNearestIntersection(shadowRay, shadowIntersection, true);
     
    bool inShadow = (shadowIntersection.hit && shadowIntersection.t < lightDistance);
    if (inShadow) {
	     if ( statistic::stat.shadow_buffer_is_on ) shadow_buffer[depth] = shadowIntersection.object;
    }
    return inShadow;
}

template<typename T, typename ObjectType>
Color<T> Raytracer<T, ObjectType>::castRay(const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, int depth, T current_density) {
	if (depth >= MAX_DEPTH) return BACKGROUND_COLOR;
    ObjectIntersection<T, 3, ObjectType> intersection = { Intersection<T, 3>{false, std::numeric_limits<T>::max(), Vector<T, 3>(), 0, 0}, nullptr};
        
    intersection = spatialStructure->findNearestIntersection(ray, intersection, false, depth);
    
    if (!intersection.hit) return BACKGROUND_COLOR;

    Vector<T, 3> hitPoint = intersection.getIntersectionPoint(ray);
    
	// Use the material from the nearest object
    auto material = std::static_pointer_cast<Material<T, 3>>(intersection.object->getMaterial());
    if (!material) {
        return BACKGROUND_COLOR;  
    }
	

	// Determine if the ray is entering or exiting the object, maybe move it to intersection since it is part of (optical) geometry
    T dotProduct = ray.get_direction().dot(intersection.normal);
    bool entering = dotProduct < 0.0f;
	if (!entering) {// If the ray is exiting, flip the normal
        intersection.normal = T(-1) * intersection.normal;
    }
	assert(material->opticalDensity > 0);
	assert(current_density > 0);
    T n1 = entering ? current_density : material->opticalDensity;
    T n2 = entering ? material->opticalDensity : current_density;
    
	// selecting which of the following components will used.
	// all (whitted style) or one random choice of the last three (path tracing)
    std::tuple<bool, bool, bool, bool> lightning_type = choose_lightning(*material);
    Color<T> resultColor{0,0,0};
	
	if (depth == 0 || std::get<1>(lightning_type)) {
        resultColor = resultColor + material->emission * material->color;
    }

    if (std::get<0>(lightning_type)) {
        resultColor = resultColor + ambientLighting(hitPoint, intersection, ray, *material);
    }
	if (std::get<1>(lightning_type)) {
        resultColor = resultColor + diffuseLighting(hitPoint, intersection, ray, lights, *material, depth, current_density);
    }
	if (std::get<2>(lightning_type)) {
		resultColor = resultColor +   reflection(hitPoint, intersection, ray, lights, depth, current_density, *material);
    }
	if (std::get<3>(lightning_type)) {
		resultColor = resultColor + transmission(hitPoint, intersection, ray, lights, depth, current_density, *material, n1, n2);
    }

    return resultColor;


}

template<typename T, typename ObjectType>
void Raytracer<T, ObjectType>::processPixel(int x, int y, const Camera<T>& camera, const std::vector<Light<T, 3>>& lights, Image& image, float gamma) {
    std::vector<Ray<T, 3>> rays = camera.generateRay(x, y, image.width, image.height, NUM_SAMPLES);
    Color<T> pixelColor(0.0f, 0.0f, 0.0f);

    for (const Ray<T, 3>& ray : rays) {
        pixelColor = pixelColor + castRay(ray, lights, 0, 1.0);
    }

    // Average color for anti-aliasing (if multiple rays)
    pixelColor = pixelColor * (1.0f / NUM_SAMPLES);

    pixelColor = Color<T>(
        std::pow(pixelColor.getRed(), gamma),
        std::pow(pixelColor.getGreen(), gamma),
        std::pow(pixelColor.getBlue(), gamma)
    );

    // Clamp color values to [0, 1]
    pixelColor = Color<T>(
        std::min(1.0f, std::max(0.0f, pixelColor.getRed())),
        std::min(1.0f, std::max(0.0f, pixelColor.getGreen())),
        std::min(1.0f, std::max(0.0f, pixelColor.getBlue()))
    );

    // Convert to 0-255 range and store in image
    unsigned int index = (y * image.width + x) * 3;
    image.pixels[index]     = static_cast<unsigned int>(pixelColor.getRed()   * image.max_color_value);
    image.pixels[index + 1] = static_cast<unsigned int>(pixelColor.getGreen() * image.max_color_value);
    image.pixels[index + 2] = static_cast<unsigned int>(pixelColor.getBlue()  * image.max_color_value);
}

template<typename T, typename ObjectType>
void Raytracer<T, ObjectType>::render(const Camera<T>& camera, const std::vector<Light<T, 3>>& lights, Image& image) {
	T gamma = 1.0 / statistic::stat.gamma_value;
    
    // High-precision clock to measure time
    auto start = std::chrono::high_resolution_clock::now();

    double totalLines = static_cast<double>(image.height);
    double processedLines = 0.0;

    for (int y = 0; y < image.height; ++y) {
        //Iterate between the widht
        for (int x = 0; x < image.width; ++x) {
            //Proccess each pixel
            processPixel(x, y, camera, lights, image, gamma); // Process each pixel
        }
		processedLines++;
		//Print code if percentage is to high to write.
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
		double secondsPerPixel = duration.count() / processedLines / 1000000.0;
		double estimatedRemainingSeconds = secondsPerPixel * (totalLines - processedLines);

		//Calculate percentage and time remaining
		double percentageComplete = ((double) (y + 1) ) / totalLines * 100.0;
		double secondsRatio = duration.count() / 1000000.0;

		// Print a progress update message.
		std::cout << "\rProgress: "
				  << std::fixed << std::setprecision(2)
				  << percentageComplete << "%"
				  << ", Estimated Time Remaining: "
				  << std::setprecision(2) << estimatedRemainingSeconds << " sec"
				  << std::flush; // Ensure output is flushed immediately
    }

    // Ensure the final message is displayed
    std::cout << std::endl;
}


// Explicit template instantiation for common types if not already done elsewhere
template class Raytracer<float, Sphere3f>;
template class Raytracer<float, Triangle3f>;
template class Raytracer<float, TriangleWithNormals3f>;