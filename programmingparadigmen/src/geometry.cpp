#include "geometry.h"
#include "statistics.h"
#include <iostream>
#include <cmath>
#include <limits>

namespace geom {

template<typename T, size_t Dim>
Vector<T, Dim> Ray<T,Dim>::get_origin() const {
	return origin;
}

template<typename T, size_t Dim>	
Vector<T, Dim> Ray<T,Dim>::get_direction() const {
	return direction;
}
	
template<typename T, size_t Dim>
bool AABB<T, Dim>::all_less_than_or_equal(const Vector<T, Dim>& a, const Vector<T, Dim>& b) {
    for (size_t i = 0; i < Dim; ++i) {
        if (a[i] > b[i]) return false;
    }
    return true;
}

template<typename T, size_t Dim>
Intersection<T, Dim> AABB<T, Dim>::intersects(const Ray<T, Dim>& ray, T min_t) const {
    Intersection<T, Dim> result = {false, 0, Vector<T, Dim>(), 0, 0}; // Initialize with default values

    T tmin = std::numeric_limits<T>::lowest();
    T tmax = std::numeric_limits<T>::max();
    size_t hitIndex = Dim; // Will hold the index of the axis where the nearest hit occurs

    for (size_t i = 0; i < Dim; ++i) {
        if (ray.get_direction()[i] == 0) {
            // Ray is parallel to this dimension, check if it's within the box on this axis
            if (ray.get_origin()[i] < min[i] || ray.get_origin()[i] > max[i]) return result; // No intersection if outside
            continue;
        }

        T invD = 1.0 / ray.get_direction()[i];
        T t0 = (min[i] - ray.get_origin()[i]) * invD;
        T t1 = (max[i] - ray.get_origin()[i]) * invD;

        if (invD < 0.0) std::swap(t0, t1);


        if (t0 > tmin) {
            tmin = t0;
            hitIndex = i;
        }
        tmax = std::min(tmax, t1); // instead: if (t1 < tmax) tmax = t1;

        if (tmin > tmax || tmin > min_t) return result; // No (better) intersection
    }


    // Intersection occurred
    result.hit = true;
    result.t = tmin; // The nearest hit distance

    // Correct the normal calculation
    result.normal = Vector<T, Dim>();
    if (hitIndex < Dim) { // Ensure hitIndex is valid
        result.normal[hitIndex] = ray.get_direction()[hitIndex] > 0 ? 1 : -1;
    }

    return result;
}

  template<typename T, size_t Dim>
  T AABB<T, Dim>::getSize() const {
        if constexpr (Dim == 2) {
            // For 2D, calculate the perimeter (analogous to surface area)
            T width = max[0] - min[0];
            T height = max[1] - min[1];
            return 2 * (width + height);  // Perimeter
        } else if constexpr (Dim == 3) {
            // For 3D, return the surface area
            T width = max[0] - min[0];
            T height = max[1] - min[1];
            T depth = max[2] - min[2];
            return 2 * (width * height + width * depth + height * depth);  // Surface Area
        } else {
            // For other dimensions, return a reasonable metric (product of extents)
            T size = 1.0;
            for (size_t i = 0; i < Dim; ++i) {
                size *= (max[i] - min[i]);
            }
            return size;
        }
    }

// Sphere intersection method
template<typename T, size_t Dim>
Intersection<T, Dim> Sphere<T, Dim>::intersect(const Ray<T, Dim>& ray, T min_t) const {
    Intersection<T, Dim> result = {false, 0, Vector<T, Dim>(), 0, 0};

    Vector<T, Dim> oc = ray.get_origin() - center;
    T a = ray.get_direction().dot(ray.get_direction());
    T b = 2.0 * oc.dot(ray.get_direction());
    T c = oc.dot(oc) - radius * radius;
    T discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return result;
    }

    T sqrtD = std::sqrt(discriminant);
    T t1 = (-b - sqrtD) / (2.0 * a);
    T t2 = (-b + sqrtD) / (2.0 * a);

    T t;
    if (t1 > 0 && t2 > 0) {
        t = std::min(t1, t2);
    } else if (t1 > 0) {
        t = t1;
    } else if (t2 > 0) {
        t = t2;
    } else {
        return result;
    }
    if (t > min_t) {
		return result;
	}

	result.t = t;
    result.hit = true;
    Vector<T, Dim> hitPoint = ray.get_origin() + ray.get_direction() * result.t;
    result.normal = (hitPoint - center).normalized();
    // For simplicity, u and v are set to zero; for actual texture mapping, you'd need to implement this.
    result.u = 0;
    result.v = 0;

    return result;
}

template<typename U, size_t D>
std::ostream& operator<<(std::ostream& os, const Sphere<U, D>& sphere) {
	os << "Sphere(center = " << sphere.center << ", radius = " << sphere.radius << ")";
    return os;
}

template<typename T, size_t Dim>
TriangleIntersectionPolicy<T, Dim> * TriangleIntersectionPolicy<T, Dim>::currentIntersectionPolicy;


template<typename T, size_t Dim>
Intersection<T, Dim> TriangleIntersectionPolicy<T,Dim>::intersect(const Triangle<T,Dim> * triangle, const Ray<T, Dim>& ray, T min_t) const {
	Intersection<T, Dim> result = {false, 0, Vector<T, Dim>(), 0, 0};
    auto edge1 = triangle->vertices[1] - triangle->vertices[0];
	auto edge2 = triangle->vertices[2] - triangle->vertices[0];
	
    auto normal =  edge1.cross(edge2);
    
    T normalRayProduct = normal.dot( ray.get_direction() );
    T area = normal.length(); // used for u-v-parameter calculation
	
    if ( fabs(normalRayProduct) < 1e-6 ) {
      return result;
    }

    T d = normal.dot( triangle->vertices[0] );
    result.t = (d - normal.dot( ray.get_origin() ) ) / normalRayProduct;

    if ( result.t < 0.0 ) {
      return result;
    }
   
    auto intersection = ray.get_origin() + result.t * ray.get_direction();
    
	auto vector = edge1.cross(intersection - triangle->vertices[0] );
    if ( normal.dot(vector) < 0.0 ) { 
      return result;
    }
	
    result.v = vector.length()  / area;

    vector = (triangle->vertices[2] - triangle->vertices[1]).cross(intersection - triangle->vertices[1] );
    if ( normal.dot(vector) < 0.0 ) { 
      return result;
    }


    vector = (triangle->vertices[0] - triangle->vertices[2]).cross(intersection - triangle->vertices[2] );
	if (normal.dot(vector) < 0.0 ) {
      return result;
    }
	
    result.u = vector.length() / area;

	result.normal = normal.normalized();
    result.hit = true;
    return result;
}



template<typename T, size_t Dim>
Intersection<T, Dim> Triangle<T, Dim>::intersect(const Ray<T, Dim>& ray, T min_t) const {
	return TriangleIntersectionPolicy<T, Dim>::currentIntersectionPolicy->intersect(this, ray, min_t);
}
  

// Triangle intersection method with per-vertex normals
template<typename T, size_t Dim>
Intersection<T, Dim> TriangleWithNormals<T, Dim>::intersect(const Ray<T, Dim>& ray, T min_t) const {
    Intersection<T, Dim> result = Triangle<T, Dim>::intersect(ray, min_t);
	
	if (result.hit) {
	    result.normal = (1 - result.u - result.v) * getNormal(0) + result.u * getNormal(1) + result.v * getNormal(2);
        result.normal = result.normal.normalized(); // Ensure the normal is unit length
	}
    return result;
}

template<typename T, size_t Dim>
Vector<T, Dim> Sphere<T, Dim>::getCentroid() const {
    return center;
}

template<typename T, size_t Dim>
Vector<T, Dim> Triangle<T, Dim>::getCentroid() const {
    return (vertices[0] + vertices[1] + vertices[2]) / static_cast<T>(3);
}

template<typename T, size_t Dim>
AABB<T, Dim> Sphere<T, Dim>::getAABB() const {
    Vector<T, Dim> min = center - Vector<T, Dim>(radius);
    Vector<T, Dim> max = center + Vector<T, Dim>(radius);
    return AABB<T, Dim>(min, max);
}



template<typename T, size_t Dim>
AABB<T, Dim> Triangle<T, Dim>::getAABB() const {
    Vector<T, Dim> min = vertices[0];
    Vector<T, Dim> max = vertices[0];
    for (size_t i = 1; i < 3; ++i) {
        for (size_t j = 0; j < Dim; ++j) {
            min[j] = std::min(min[j], vertices[i][j]);
            max[j] = std::max(max[j], vertices[i][j]);
        }
    }
    return AABB<T, Dim>(min, max);
}

template<typename T, size_t Dim>
Vector<T, Dim> TriangleWithNormals<T, Dim>::getCentroid() const {
    return Triangle<T, Dim>::getCentroid(); 
}

template<typename T, size_t Dim>
Vector<T, Dim> Intersection<T, Dim>::getIntersectionPoint(const Ray<T, Dim>& ray) const {
    return ray.get_origin() + t * ray.get_direction();
}


template<typename F> 
std::vector<Triangle<F, 3>> create_prisma(
	     Vector<F, 3> corner, Vector<F, 3> right, Vector<F, 3> left, Vector<F, 3> up,
         bool ccw,
		 const std::shared_ptr<BaseMaterial>& material) {
    // Compute the 8 vertices of the prism
    Vector<F, 3> v0 = corner;
    Vector<F, 3> v1 = corner + right;
    Vector<F, 3> v2 = corner + left;
    Vector<F, 3> v3 = corner + left + right;
    Vector<F, 3> v4 = corner + up;
    Vector<F, 3> v5 = corner + up + right;
    Vector<F, 3> v6 = corner + up + left + right;
    Vector<F, 3> v7 = corner + up + left;
    std::vector<Triangle<F, 3>> triangles;

    if (ccw) {
        // Front face
        triangles.emplace_back(v0, v1, v5, ccw, material);
        triangles.emplace_back(v5, v4, v0, ccw, material);
        // Back face
        triangles.emplace_back(v3, v2, v6, ccw, material);
        triangles.emplace_back(v6, v2, v7, ccw, material);
        // Base
        triangles.emplace_back(v0, v2, v3, ccw, material);
        triangles.emplace_back(v3, v1, v0, ccw, material);
        // Top
        triangles.emplace_back(v4, v5, v6, ccw, material);
        triangles.emplace_back(v6, v7, v4, ccw, material);
        // Right face
        triangles.emplace_back(v1, v3, v6, ccw, material); 
        triangles.emplace_back(v6, v5, v1, ccw, material);
        // Left face
        triangles.emplace_back(v7, v2, v0, ccw, material);
        triangles.emplace_back(v0, v4, v7, ccw, material);
    } else {
        // Front face
        triangles.emplace_back(v1, v0, v5, ccw, material);
        triangles.emplace_back(v4, v5, v0, ccw, material);
        // Back face
        triangles.emplace_back(v2, v3, v6, ccw, material);
        triangles.emplace_back(v2, v6, v7, ccw, material);
        // Base
        triangles.emplace_back(v2, v0, v3, ccw, material);
        triangles.emplace_back(v1, v3, v0, ccw, material);
        // Top
        triangles.emplace_back(v5, v4, v7, ccw, material);
        triangles.emplace_back(v7, v6, v4, ccw, material);
        // Right face
        triangles.emplace_back(v3, v1, v6, ccw, material); 
        triangles.emplace_back(v5, v6, v1, ccw, material);
        // Left face
        triangles.emplace_back(v2, v7, v0, ccw, material);
        triangles.emplace_back(v4, v0, v7, ccw, material);
    }

    return triangles;
}

}// end namespace