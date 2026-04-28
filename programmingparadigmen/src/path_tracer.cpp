#include "path_tracer.h"
#include "statistics.h"

template<typename T, typename ObjectType>
std::tuple<bool, bool, bool, bool> PathTracer<T, ObjectType>::choose_lightning(const Material<T, 3>& material) {
    T diffuseProb = material.kd;
    T reflectProb = material.reflectivity;
    T transmitProb = material.transmission;
    T ambientProb = material.ka;

    T sum = diffuseProb + reflectProb + transmitProb + ambientProb;
	
    diffuseProb /= sum;
    reflectProb /= sum;
    transmitProb /= sum;
    ambientProb /= sum;

    T rnd = static_cast<T>(rand()) / RAND_MAX;

    if (rnd < ambientProb) {
        return std::make_tuple(true, false, false, false);
    } else if (rnd < ambientProb + diffuseProb) {
        return std::make_tuple(false, true, false, false);
    } else if (rnd < ambientProb + diffuseProb + reflectProb) {
        return std::make_tuple(false, false, true, false);
    } else {
        return std::make_tuple(false, false, false, true);
    }

}

template<typename T, typename ObjectType>
Color<T> PathTracer<T, ObjectType>::ambientLighting(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const Material<T, 3>& material) {
    return material.color * (material.ka + material.emission) ; // Ambient lighting
}

/*
template<typename T, typename ObjectType>
Vector<T, 3> generateRandomHemisphereDirection(const Vector<T, 3>& normal) {
    T r1 = static_cast<T>(rand()) / RAND_MAX;
    T r2 = static_cast<T>(rand()) / RAND_MAX;
    T sinTheta = std::sqrt(r1);
    T cosTheta = std::sqrt(1.0 - r1);
    T phi = 2 * M_PI * r2;

    Vector<T, 3> tangent, bitangent;
    if (std::abs(normal[1]) > std::abs(normal[0])) {
        tangent = Vector<T, 3>{-normal[2], 0, normal[0]};
		std::cout << "x " << tangent << std::endl;
		tangent = tangent.normalized();
		std::cout << "ok" << std::endl;
    } else {
        tangent = Vector<T, 3>{0, -normal[2], normal[1]};
		std::cout << "y " << tangent << std::endl;
		tangent = tangent.normalized();
		std::cout << "ok" << std::endl;
    }
    bitangent = normal.cross(tangent);
    std::cout << "z " << (tangent * std::cos(phi) * sinTheta + bitangent * std::sin(phi) * sinTheta + normal * cosTheta) << std::endl;
	(tangent * std::cos(phi) * sinTheta + bitangent * std::sin(phi) * sinTheta + normal * cosTheta).normalized();
	std::cout << "ok" << std::endl;
    return (tangent * std::cos(phi) * sinTheta + bitangent * std::sin(phi) * sinTheta + normal * cosTheta).normalized();
}
*/

template<typename T, typename ObjectType>
Vector<T, 3> generateRandomHemisphereDirection(const Vector<T, 3>& normal) {
    T r1 = static_cast<T>(rand()) / RAND_MAX;
    T r2 = static_cast<T>(rand()) / RAND_MAX;
    T sinTheta = std::sqrt(r1);
    T cosTheta = std::sqrt(1.0 - r1);
    T phi = 2 * M_PI * r2;

    // Wähle einen Vektor, der nicht parallel zur Normalen ist
    Vector<T, 3> temp = (std::abs(normal[2]) < 0.9) ? Vector<T, 3>{0, 0, 1} : Vector<T, 3>{1, 0, 0};
    Vector<T, 3> tangent = normal.cross(temp).normalized();
    Vector<T, 3> bitangent = normal.cross(tangent); // Bereits normalisiert, da normal und tangent orthonormal sind

    return (tangent * std::cos(phi) * sinTheta + bitangent * std::sin(phi) * sinTheta + normal * cosTheta).normalized();
}

template<typename T, typename ObjectType>
Ray<T, 3> scatteredRay(const Vector<T, 3>& hitPoint, const Vector<T, 3>& normal) {
    Vector<T, 3> randomDir = generateRandomHemisphereDirection(normal);
    return Ray<T, 3>(hitPoint + normal * T(0.001), randomDir);
}

template<typename T, typename ObjectType>
Color<T> PathTracer<T, ObjectType>::diffuseLighting(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, const Material<T, 3>& material, const int depth, T current_density) {
    //Color<T> emittedLight = material.emission * material.color;
	
	// Generate a random direction in the hemisphere around the normal
    Vector<T, 3> randomDir = generateRandomHemisphereDirection<T, ObjectType>(intersection.normal);
    
    // Create a new ray for the scattered direction
    Ray<T, 3> scatteredRay(hitPoint + intersection.normal * T(0.001), randomDir);
    
    // Recursively call castRay with the scattered ray
    Color<T> indirectLight = Raytracer<T, ObjectType>::castRay(scatteredRay, lights, depth + 1, current_density);
    
    // Calculate the cosine term
    T cosTheta = std::max(T(0), intersection.normal.dot(randomDir));
    
    // Multiply by the material's diffuse color and the cosine term
	return material.kd * indirectLight * cosTheta * material.color;
}

template<typename T, typename ObjectType>
Color<T> PathTracer<T, ObjectType>::reflection(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, int depth, T current_density, const Material<T, 3>& material) {
    Vector<T, 3> reflectedDir = OpticalGeometry<T, 3>::reflect(ray, intersection.normal).normalized();
    Ray<T, 3> reflectedRay(hitPoint + intersection.normal * T(0.001), reflectedDir);
    return Raytracer<T, ObjectType>::castRay(reflectedRay, lights, depth + 1, current_density) * material.reflectivity;
}

template<typename T, typename ObjectType>
Color<T> PathTracer<T, ObjectType>::transmission(const Vector<T, 3>& hitPoint, const ObjectIntersection<T, 3, ObjectType>& intersection, const Ray<T, 3>& ray, const std::vector<Light<T, 3>>& lights, int depth, T current_density, const Material<T, 3>& material, T n1, T n2) {
    if (ray.get_direction().dot(intersection.normal) > 0) {
        std::swap(n1, n2);
    }
    Vector<T, 3> refractedDir;
    if ( OpticalGeometry<T, 3>::refract(ray, intersection.normal, n1, n2, refractedDir)) {
        Ray<T, 3> refractedRay(hitPoint - intersection.normal * T(0.001), refractedDir.normalized());
        return Raytracer<T, ObjectType>::castRay(refractedRay, lights, depth + 1, n2) * material.transmission;
    }
    return Color<T>(0, 0, 0);
}

template<typename T, typename ObjectType>
void PathTracer<T, ObjectType>::render(const Camera<T>& camera, const std::vector<Light<T, 3>>& lights, Image& image) {
	Raytracer<T, ObjectType>::render(camera, lights, image);
}

// Explicit template instantiation for common types if not already done elsewhere
template class PathTracer<float, Sphere3f>;
template class PathTracer<float, Triangle3f>;
template class PathTracer<float, TriangleWithNormals3f>;