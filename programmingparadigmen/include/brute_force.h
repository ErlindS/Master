#ifndef BRUTE_FORCE_H
#define BRUTE_FORCE_H

#include "spatial_structure.h" 

#include <vector>
#include <memory>
#include <limits>

namespace geom {

/**
 * @brief BruteForce class implementing a simple spatial structure using brute force intersection checks.
 * 
 * This class inherits from SpatialStructure but uses a straightforward, unoptimized method for 
 * finding intersections by checking all objects against each ray.
 * 
 * @tparam T The floating-point type used for vector components.
 * @tparam Dim The spatial dimension (typically 3 for 3D ray tracing).
 * @tparam ObjType The type of geometric objects stored in the structure.
 */
template<typename T, size_t Dim, typename ObjType>
class BruteForce : public SpatialStructure<T, Dim, ObjType> {
private:
    std::vector<std::shared_ptr<ObjType> > objects; ///< All objects stored for brute force intersection checking.
protected:
   /**
     * @brief Finds the nearest intersection with a given ray.
     * 
     * @param ray The ray to test for intersection.
     * @param best Reference to the best intersection found so far.
	 * @param stopOnFirstIntersection  true the first intersection is returned, not necessarly the nearest. Useful for instance for shadow rays.
     * @return Updated intersection information.
     */
    ObjectIntersection<T, Dim, ObjType> nearestIntersection(const Ray<T, Dim>& ray, ObjectIntersection<T, Dim, ObjType> best, bool stopOnFirstIntersection = false, int depth = 0) override;

public:
    /**
     * @brief Constructor for BruteForce structure.
     */
    BruteForce() = default;

    /**
     * @brief Destructor for BruteForce, ensuring proper resource management.
     */
    ~BruteForce() override = default;

    /**
     * @brief Builds the structure by storing all objects for later brute force checking.
     * 
     * @param objects Vector of shared pointers to the geometric objects to be inserted.
     * @param strategy In this case, strategy is ignored as brute force doesn't require a strategy.
     */
    void build(std::vector<std::shared_ptr<ObjType>>& objects, const std::string& strategy = "default") override;

};

// Typedef for common use case
using BruteForce3fSphere = BruteForce<float, 3, Sphere<float, 3>>;
using BruteForce3fTriangle = BruteForce<float, 3, Triangle<float, 3>>;
} // END namespace geom

#endif // BRUTE_FORCE_H
