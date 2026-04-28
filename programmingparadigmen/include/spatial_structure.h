#ifndef SPATIAL_STRUCTURE_H
#define SPATIAL_STRUCTURE_H

#include "vector.h"
#include "geometry.h"
#include <map>
#include "statistics.h"

using namespace math;

namespace geom {

/**
 * @brief Represents an intersection result that includes the intersected object.
 * 
 * This extends the standard Intersection structure by associating an intersected
 * object with the intersection data.
 * 
 * @tparam T The floating-point type used for calculations.
 * @tparam Dim The dimensionality of the space (e.g., 3 for 3D).
 * @tparam ObjType The type of object involved in the intersection.
 */
template<typename T, size_t Dim, typename ObjType>
struct ObjectIntersection : public Intersection<T, Dim> {
    std::shared_ptr<ObjType> object; ///< Pointer to the intersected object, if any.

    ObjectIntersection() : object(nullptr) {}

    ObjectIntersection(const Intersection<T, Dim>& base, std::shared_ptr<ObjType> obj = nullptr)
        : Intersection<T, Dim>(base), object(obj) {}
		
	Vector<T, Dim> getNormal() {
		return object->getNormal(*this);
	}
};

/**
 * @brief Abstract base class for spatial partitioning structures in ray tracing.
 * 
 * This class defines an interface for spatial acceleration structures, such as 
 * KD-Trees, Octrees, and BSP Trees, used to optimize ray-object intersection tests.
 * 
 * @tparam T The floating-point type used for vector components.
 * @tparam Dim The spatial dimension (typically 3 for 3D ray tracing).
 * @tparam ObjType The type of geometric objects stored in the structure.
 */
template<typename T, size_t Dim, typename ObjType>
class SpatialStructure {
protected:
    std::map<int, std::shared_ptr<ObjType>> hit_buffer_by_depth; // stores the last intersected object per (recursive) depth of for instance a raytracer

    /**
     * @brief Finds the nearest intersection of a ray with objects in the structure.
     * 
     * @param ray The ray to test for intersections.
	 * @param stopOnFirstIntersection Ff true the first intersection is returned, not necessarly the nearest. Useful for instance for shadow rays.
     * @return The closest intersection found, or an intersection with hit=false if none exists.
     */
    virtual ObjectIntersection<T, Dim, ObjType> nearestIntersection(const Ray<T, Dim>& ray, ObjectIntersection<T, Dim, ObjType> best, bool stopOnFirstIntersection = false, int depth = 0)  = 0;

public:
    /**
     * @brief Virtual destructor for safe cleanup of derived classes.
     */
    virtual ~SpatialStructure() = default;

   /**
     * @brief Updates an object in the structure. 
     * 
     * This method should adjust the spatial structure to account for changes in object size or position.
     * 
     * @param old_obj The old object to be replaced.
     * @param new_obj The new object with updated properties.
     * @return True if the object was successfully updated, false otherwise.
     */
    //virtual bool updateObject(const std::shared_ptr<ObjType>& old_obj, const std::shared_ptr<ObjType>& new_obj) = 0;

    /**
     * @brief Adds a new object to the spatial structure.
     * 
     * @param obj The new object to be added.
     * @return True if the object was successfully added, false otherwise.
     */
    //virtual bool addObject(const std::shared_ptr<ObjType>& obj) = 0;

    /**
     * @brief Removes an object from the spatial structure.
     * 
     * @param obj The object to be removed.
     * @return True if the object was successfully removed, false if the object was not found.
     */
    //virtual bool removeObject(const std::shared_ptr<ObjType>& obj) = 0;


    /**
     * @brief Constructs the spatial structure using the given objects.
     * 
     * @param objects A vector of shared pointers to the geometric objects to be inserted.
     * @param strategy An optional string specifying the build strategy (e.g., "median" for KD-Trees). "default" for a default-strategy.
     */
    virtual void build(std::vector<std::shared_ptr<ObjType>>& objects, const std::string& strategy = "default") = 0;


    ObjectIntersection<T, Dim, ObjType> findNearestIntersection(const Ray<T, Dim>& ray, Intersection<T, Dim> intersection = Intersection<T, 3>{false, std::numeric_limits<T>::max(), Vector<T, 3>(), 0, 0}, bool stopOnFirstIntersection = false, const int depth = 0) {
	    ObjectIntersection<T, Dim, ObjType> best(intersection, nullptr);

        if ( statistic::stat.hit_buffer_is_on ) {	
	        if ( this->hit_buffer_by_depth[depth] != nullptr ) {
			    statistic::stat.spatial_structure_no_intersection_tests++;
		        auto intersection = ObjectIntersection(this->hit_buffer_by_depth[depth]->intersect(ray), this->hit_buffer_by_depth[depth]);
		        if (intersection.hit && intersection.t > 0) {
			        statistic::stat.hit_buffer_hits++;
					statistic::stat.spatial_structure_no_nearer_intersection_tests++;
				    best = intersection;
					if (stopOnFirstIntersection) return best;
		        }
	        }
	    }
        best = nearestIntersection(ray, best, stopOnFirstIntersection, depth);
	    if ( statistic::stat.hit_buffer_is_on && best.hit )  this->hit_buffer_by_depth[depth] = best.object;
	    return best;
    }
	
    /**
     * @brief Performs a brute-force search for the nearest intersection with a ray.
     * 
     * This method checks every object in the given list for intersections with the ray
     * and returns the closest intersection found.
     * 
     * @param ray The ray to test.
     * @param objects A vector of shared pointers to objects to test against.
     * @param best The best intersection found so far (default: no hit, max distance).
	 * @param stopOnFirstIntersection true the first intersection is returned, not necessarly the nearest. Useful for instance for shadow rays.
     * @return The nearest intersection found, or an intersection with hit=false if none exists.
     */
static ObjectIntersection<T, Dim, ObjType> bruteForceNearestIntersection(
    const Ray<T, Dim>& ray,
    const std::vector<std::shared_ptr<ObjType>>& objects,
    ObjectIntersection<T, Dim, ObjType> best = ObjectIntersection<T, Dim, ObjType>(
        {false, std::numeric_limits<T>::max(), Vector<T, Dim>(), 0, 0}
    ),
    bool stopOnFirstIntersection = false
) {
    constexpr T EPSILON = 1e-6; // Small threshold to avoid floating-point precision issues

    for (const auto& obj : objects) {
	    statistic::stat.spatial_structure_no_intersection_tests++;
        auto objectIntersection = obj->intersect(ray, best.t);
        if (objectIntersection.hit && objectIntersection.t < best.t && objectIntersection.t > EPSILON) {
			statistic::stat.spatial_structure_no_nearer_intersection_tests++;
            best = ObjectIntersection<T, Dim, ObjType>(objectIntersection, obj);

            if (stopOnFirstIntersection) return best;
        }
    }

    return best;
}

};

// Typedefs for common use cases
using SpatialStructure3fSphere = SpatialStructure<float, 3, Sphere<float, 3>>;
using SpatialStructure3fTriangle = SpatialStructure<float, 3, Triangle<float, 3>>;

} // END namespace
#endif // SPATIAL_STRUCTURE_H
