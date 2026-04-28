#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <variant>
#include <memory>
#include <vector>
#include <limits>
#include "vector.h"

using namespace math;

namespace geom {
	
/**
 * @brief Represents a ray in n-dimensional space, used for ray tracing and intersection tests.
 * 
 * A ray is defined by an origin point \( \mathbf{o} \) and a direction vector \( \mathbf{d} \), such that
 * any point along the ray is given by the parametric equation:
 * 
 * \[ \mathbf{p}(t) = \mathbf{o} + t \mathbf{d}, \quad t \geq 0 \]
 */
template<typename T, size_t Dim>
class Ray {
    Vector<T, Dim> origin,    ///< The starting point of the ray.
	               direction; ///< The direction of the ray (should be normalized).
public:
    Vector<T, Dim> get_origin() const;
	
	Vector<T, Dim> get_direction() const;
	
    /**
     * @brief Default constructor initializes origin and direction to zero vectors.
     */
	Ray() : origin(), direction() {} 
    /**
     * @brief Constructs a ray with a given origin and direction.
     * 
     * @param o Origin of the ray.
     * @param d Direction of the ray (should be a unit vector, i.e., \( ||\mathbf{d}|| = 1 \)).
     */
	 Ray(const Vector<T, Dim>& o, const Vector<T, Dim>& d) : origin(o), direction(d) {}
};


/**
 * @brief Represents the result of a ray-object intersection test.
 * 
 * Stores information about whether an intersection occurred, the distance \( t \),
 * and the surface normal at the intersection point.
 * Texture coordinates and normals are optional values.
 */
template<typename T, size_t Dim>
struct Intersection {
    bool hit; ///< Indicates if the intersection is valid (true if intersection occurs).
    T t;      ///< Distance from ray origin to intersection point (\( t \geq 0 \)).
    Vector<T, Dim> normal;  ///< Surface normal at the intersection point (should be normalized).
    T u, v;   ///< Optional texture coordinates in range \( [0,1] \).
	
	/**
     * @brief Computes the intersection point given the intersecting ray.
     * 
     * @param ray The ray involved in the intersection.
     * @return The computed intersection point \( \mathbf{p}(t) = \mathbf{o} + t \mathbf{d} \).
     */
    Vector<T, Dim> getIntersectionPoint(const Ray<T, Dim>& ray) const;
	
};


/**
 * @brief Base class for all materials in the ray tracer.
 */
class BaseMaterial {
public:
    BaseMaterial() = default;
};

/**
 * @brief Base class for all geometric objects in the scene.
 */
template<typename T, size_t Dim>
class GeometryObject {
protected:
    std::shared_ptr<BaseMaterial> material; ///< Pointer to the material of the object.

public:
    /**
     * @brief Default constructor initializes with a default material.
     */
	GeometryObject() : material(std::make_shared<BaseMaterial>()) {}
    
	/**
     * @brief Constructor with a specified material.
     * @param mat Shared pointer to a material instance.
     */
	 GeometryObject(const std::shared_ptr<BaseMaterial>& mat) : material(mat) {}
    
    /**
     * @brief Retrieves the material of the object.
     * @return Shared pointer to the material.
     */
	 std::shared_ptr<BaseMaterial> getMaterial() const { return material; }
	 
	Vector<T, Dim> getNormal(Intersection<T, Dim> & intersection) { return intersection.normal;}
};

/**
 * @brief Represents an Axis-Aligned Bounding Box (AABB) in n-dimensional space.
 * 
 * The AABB is defined by its minimum \( \mathbf{min} \) and maximum \( \mathbf{max} \) corner points.
 */
template<typename T, size_t Dim>
class AABB {
public:
    Vector<T, Dim> min; ///< Minimum corner of the AABB.
    Vector<T, Dim> max; ///< Maximum corner of the AABB.
    
	AABB() = default;
    /**
     * @brief Constructs an AABB given min and max points.
     * @param min The minimum corner.
     * @param max The maximum corner (must satisfy \( \mathbf{min} \leq \mathbf{max} \) component-wise).
     */
    AABB(const Vector<T, Dim>& min, const Vector<T, Dim>& max) : min(min), max(max) {
        assert(all_less_than_or_equal(min, max));
    }
    
	 /**
     * @brief Checks if the AABB intersects with the given ray.
     * 
     * @param ray The ray to test for intersection with the AABB.
	 * @param min_t The current value t value for a previously found intersection.
     * @return An Intersection structure containing all details except texture coordinates about the intersection if it occurs.
     */
    Intersection<T, Dim> intersects(const Ray<T, Dim>& ray, T min_t = std::numeric_limits<T>::max()) const;
	
	/**
     * @brief Returns the size (volume, area or similar) of the AABB.
     *
     * @return T the size (volume, area or similar) of the AABB.
     */
    T getSize() const;
	
private:
    static bool all_less_than_or_equal(const Vector<T, Dim>& a, const Vector<T, Dim>& b);
};

/**
 * @brief Represents a sphere in n-dimensional space.
 * 
 * Defined by a center \( \mathbf{c} \) and radius \( r \).
 */
template<typename T, size_t Dim>
class Sphere : public GeometryObject<T, Dim> {
public:
    Vector<T, Dim> center; ///< Center of the sphere.
    T radius; ///< Radius (must be \( r > 0 \)).
	
    /**
     * @brief Constructs a sphere with a center and radius.
     * @param c Center of the sphere.
     * @param r Radius (\( r > 0 \)).
     * @param mat Optional material.
     */
	 Sphere(const Vector<T, Dim>& c, T r, const std::shared_ptr<BaseMaterial>& mat = std::make_shared<BaseMaterial>()) 
        : GeometryObject<T, Dim>(mat), center(c), radius(r) {}
		
	/**
     * @brief Tests if this sphere intersects with the given ray.
     * 
     * @param ray The ray to test for intersection.
	 * @param min_t The current value t value for a previously found intersection.
     * @return An Intersection structure describing the intersection details except texture coordinates.
     */
    Intersection<T, Dim> intersect(const Ray<T, Dim>& ray, T min_t = std::numeric_limits<T>::max()) const;

    /**
     * @brief Returns the centroid of the sphere, which is its center.
     * 
     * For a sphere, the centroid is the same as its center.
     * 
     * @return Vector<T, Dim> representing the center/centroid of the sphere.
     */
    Vector<T, Dim> getCentroid() const;
	
   /**
     * @brief Returns the Axis-Aligned Bounding Box of the sphere.
     * 
     * @return AABB<T, Dim> The bounding box of the sphere.
     */
    AABB<T, Dim> getAABB() const;
	

   /**
     * @brief Outputs the sphere in a human-readable format to an output stream.
     * @param os The output stream to write to.
     * @param vec The sphere to output.
     * @return Reference to the output stream.
     */
    template<typename U, size_t D>
    friend std::ostream& operator<<(std::ostream& os, const Sphere<U, D>& sphere);
};

/**
 * @brief Represents a triangle in n-dimensional space with vertices and orientation.
 * 
 * This class is used for geometric computations, particularly ray-triangle intersections.
 */
template<typename T, size_t Dim>
class Triangle : public GeometryObject<T, Dim> {
public:
    Vector<T, Dim> vertices[3];
			
    bool ccw; ///< Indicates if the vertices are ordered counter-clockwise.
	
     /**
     * @brief Constructor for Triangle.
     * 
     * @param v1 First vertex of the triangle.
     * @param v2 Second vertex of the triangle.
     * @param v3 Third vertex of the triangle.
     * @param is_ccw Whether the vertices are in counter-clockwise order (default true).
	 * @param mat The material of this object.
     */
	 Triangle(const Vector<T, Dim>& v1, const Vector<T, Dim>& v2, const Vector<T, Dim>& v3, 
             bool is_ccw = true, const std::shared_ptr<BaseMaterial>& mat = std::make_shared<BaseMaterial>()) 
        : GeometryObject<T, Dim>(mat), vertices{v1, v2, v3}, ccw(is_ccw) {}
		
	/**
     * @brief Determines if and where this triangle intersects with the given ray.
     * 
     * @param ray The ray to check for intersection.
	 * @param min_t The current value t value for a previously found intersection.
     * @return An Intersection structure with all intersection details if it occurs.
     */	
    Intersection<T, Dim> intersect(const Ray<T, Dim>& ray, T min_t = std::numeric_limits<T>::max()) const;

    /**
     * @brief Calculates the centroid of the triangle.
     * 
     * The centroid is the average position of all vertices.
     * 
     * @return Vector<T, Dim> representing the centroid of the triangle.
     */
    Vector<T, Dim> getCentroid() const;
	
    /**
     *  @brief Computes the AABB for the triangle.
     * 
     * @return AABB<T, Dim> The bounding box of the triangle.
     */
    AABB<T, Dim> getAABB() const;
	
};	

/**
 * @brief Represents an ray triangle intersection algorithm that can be used
 *  to change the intersection algorithm of Triangle.
 * 
 * Create your own intersection policy by overriding this class and
 * setting currentIntersectionPolicy to an instance of your class.
 */
template<typename T, size_t Dim>
class TriangleIntersectionPolicy {
public:
   TriangleIntersectionPolicy() = default;
   static TriangleIntersectionPolicy<T, Dim> * currentIntersectionPolicy;
   virtual Intersection<T, Dim> intersect(const Triangle<T,Dim> * triangle, const Ray<T, Dim>& ray, T min_t = std::numeric_limits<T>::max()) const;
};

/**
 * @brief Represents a triangle with per-vertex normals for more accurate normal calculations.
 * 
 * This extends the basic Triangle class by adding normals at each vertex for better surface approximation.
 */
template<typename T, size_t Dim>
class TriangleWithNormals : public Triangle<T, Dim> {
public:
    Vector<T, Dim> normals[3]; // One normal per vertex	
	
	/**
     * @brief Constructor for TriangleWithNormals.
     * 
     * @param v1 First vertex of the triangle.
     * @param v2 Second vertex of the triangle.
     * @param v3 Third vertex of the triangle.
     * @param n1 Normal at the first vertex.
     * @param n2 Normal at the second vertex.
     * @param n3 Normal at the third vertex.
     * @param is_ccw Whether the vertices are in counter-clockwise order (default true).
	 * @param mat The material of this object.
     */
    TriangleWithNormals(const Vector<T, Dim>& v1, const Vector<T, Dim>& v2, const Vector<T, Dim>& v3, 
                        const Vector<T, Dim>& n1, const Vector<T, Dim>& n2, const Vector<T, Dim>& n3, 
                        bool is_ccw = true, const std::shared_ptr<BaseMaterial>& mat = std::make_shared<BaseMaterial>()) 
        : Triangle<T, Dim>(v1, v2, v3, is_ccw, mat) , normals{ n1, n2, n3} {
		}
    
	TriangleWithNormals(const Triangle<T, Dim> & triangle) : Triangle<T, Dim>(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], triangle.ccw, triangle.getMaterial())
	{
	   auto normal = (this->vertices[1] - this->vertices[0]).cross(this->vertices[2] - this->vertices[0]).normalized();
	   normals[0] = normals[1] = normals[2] = normal;
	}
	/**
     * @brief Determines if and where this triangle with normals intersects with the given ray.
     * 
     * This method uses the per-vertex normals for computing the intersection normal.
     * 
     * @param ray The ray to check for intersection.
	 * @param min_t The current value t value for a previously found intersection.
     * @return An Intersection structure with intersection details including interpolated normal and texture coordinates.
     */
    Intersection<T, Dim> intersect(const Ray<T, Dim>& ray, T min_t = std::numeric_limits<T>::max()) const;
	
	    /**
     * @brief Calculates the centroid of the triangle.
     * 
     * The centroid is the same as the parent Triangle class, representing the average position of all vertices.
     * 
     * @return Vector<T, Dim> representing the centroid of the triangle.
     */
    Vector<T, Dim> getCentroid() const;
	
	Vector<T, Dim> getNormal(size_t i) const {
		return normals[i];
	}
		
	Vector<T, Dim> & getNormal(size_t i)  {
        return normals[i];
	}
	
	Vector<T, Dim> getNormal(Intersection<T, Dim> & intersection) const {
	    intersection.normal = (1 - intersection.u - intersection.v) * getNormal(0) + intersection.u * getNormal(1) + intersection.v * getNormal(2);
        intersection.normal = intersection.normal.normalized(); // Ensure the normal is unit length
		return intersection.normal;
	}
};

/**
 * @brief A generic shape class that wraps different geometric objects.
 * 
 * This templated class extends GeometryObject and can store a Sphere, Triangle, 
 * or TriangleWithNormals using a variant. It delegates method calls to the stored object.
 *
 * @tparam T The numerical type (e.g., float, double).
 * @tparam Dim The dimensionality of the geometry (e.g., 2D, 3D).
 */
template<typename T, size_t Dim>
class Shape : public GeometryObject<T, Dim> {
public:
    /**
     * @brief Enum representing the type of geometric object stored in Shape.
     */
    enum class ShapeType { AABB, SPHERE, TRIANGLE, TRIANGLE_WITH_NORMALS };

private:
    std::variant<AABB<T, Dim>, Sphere<T, Dim>, Triangle<T, Dim>, TriangleWithNormals<T, Dim>> object; ///< The stored geometric object.
    ShapeType type; ///< The type of the stored object.

public:
    /**
     * @brief Constructs a Shape from a AABB.
     * @param sphere The aabb to store.
     */
    Shape(const AABB<T, Dim>& aabb) : object(aabb), type(ShapeType::AABB) {}

   /**
     * @brief Constructs a Shape from a Sphere.
     * @param sphere The sphere to store.
     */
    Shape(const Sphere<T, Dim>& sphere) : object(sphere), type(ShapeType::SPHERE) {}

    /**
     * @brief Constructs a Shape from a Triangle.
     * @param triangle The triangle to store.
     */
    Shape(const Triangle<T, Dim>& triangle) : object(triangle), type(ShapeType::TRIANGLE) {}

    /**
     * @brief Constructs a Shape from a TriangleWithNormals.
     * @param triangleWithNormals The triangle with normals to store.
     */
    Shape(const TriangleWithNormals<T, Dim>& triangleWithNormals) 
        : object(triangleWithNormals), type(ShapeType::TRIANGLE_WITH_NORMALS) {}

    /**
     * @brief Computes the intersection of the shape with a given ray.
     * @param ray The ray to check for intersection.
	 * @param min_t The current value t value for a previously found intersection.
     * @return The intersection details, if any.
     */
    Intersection<T, Dim> intersect(const Ray<T, Dim>& ray, T min_t = std::numeric_limits<T>::max()) const override {
        return std::visit([&](const auto& obj) { return obj.intersect(ray); }, object);
    }

    /**
     * @brief Computes the centroid of the stored geometric object.
     * @return The centroid of the object as a vector.
     */
    Vector<T, Dim> getCentroid() const override {
        return std::visit([](const auto& obj) { return obj.getCentroid(); }, object);
    }

    /**
     * @brief Computes the axis-aligned bounding box (AABB) of the stored object.
     * @return The AABB of the object as a vector.
     */
    Vector<T, Dim> getAABB() const override {
        return std::visit([](const auto& obj) { return obj.getAABB(); }, object);
    }

    /**
     * @brief Gets the type of the stored geometric object.
     * @return The type of the object (Sphere, Triangle, or TriangleWithNormals).
     */
    ShapeType getType() const { return type; }
};


	/**
	 @brief Creates the prism shaped object made from a triangles.
	 
	 Creates a prism like shaped object made from triangles with faces to the outside.
	 The faces are to the outside when the orientation of the triangles vertices are ccw oriented (if ccw is true)
	 or counter ccw (if ccw is false).
	 
	         7-------6
	        /       / \
	       /   \   /   \
	      /       /     \
	     4-------5       \
		  \       \       \
	       \      2- - - -3
            \     /	\     /
	         \       \   /
              \ /     \ /
              0--------1	
			  
      Where the vertices calculated as:
      0 = corner
      1 = corner + right
      2 = corner + left
      3 = corner + left + right
      4 = corner + up
      5 = corner + up + right
      6 = corner + up + left + right
      7 = corner + up + left	

     The return value contains the triangles in the order (here vertices are in ccw order, face normals pointing away from the triangles)
        0, 1, 5; 5, 4, 0; (front)
		3, 2, 6; 6, 2, 7; (back) 
	    0, 2, 3; 3, 1, 0; (base)
		4, 5, 6; 6, 7, 4; (top)
		1, 3, 6; 6, 5, 1; (right side)
		7, 2, 0; 0, 4, 7; (left side)
	
     The orientation and thus the normals are not correct if the the corner (resp. left, right, up) are
      not given as depicted.	 
     @return std::vector<Triangle<F, 3> The triangles forming the prisma. 		  
	 */
	template<typename F> 
	 std::vector<Triangle<F, 3>> create_prisma(
	     Vector<F, 3> corner, Vector<F, 3> right, Vector<F, 3> left, Vector<F, 3> up,
         bool ccw = true,
		 const std::shared_ptr<BaseMaterial>& material = std::make_shared<BaseMaterial>());


// Typedefs for common types
typedef AABB<float, 2> AABB2f;
typedef AABB<float, 3> AABB3f;
typedef Sphere<float, 2> Sphere2f;
typedef Sphere<float, 3> Sphere3f;
typedef Triangle<float, 2> Triangle2f;
typedef Triangle<float, 3> Triangle3f;
typedef TriangleWithNormals<float, 2> TriangleWithNormals2f;
typedef TriangleWithNormals<float, 3> TriangleWithNormals3f;
typedef TriangleIntersectionPolicy<float, 2> TriangleIntersectionPolicy2f;
typedef TriangleIntersectionPolicy<float, 3> TriangleIntersectionPolicy3f;
typedef Ray<float, 2> Ray2f;
typedef Ray<float, 3> Ray3f;
typedef Shape<float, 2> Shape2f;
typedef Shape<float, 3> Shape3f;
} // end namespace
#endif // GEOMETRY_H