#include "brute_force.h"

namespace geom {

template<typename T, size_t Dim, typename ObjType>
void BruteForce<T, Dim, ObjType>::build(std::vector<std::shared_ptr<ObjType>>& objects, const std::string& strategy) {
    this->objects = objects; // Simply store all objects for brute force search
}

template<typename T, size_t Dim, typename ObjType>
ObjectIntersection<T, Dim, ObjType> BruteForce<T, Dim, ObjType>::nearestIntersection(const Ray<T, Dim>& ray, ObjectIntersection<T, Dim, ObjType> best, bool stopOnFirstIntersection, int depth)  {
    for (const auto& obj : this->objects) {
		statistic::stat.spatial_structure_no_intersection_tests++;
        auto objectIntersection = obj->intersect(ray, best.t);
        if (objectIntersection.hit && objectIntersection.t < best.t && objectIntersection.t > T(0)) {
            best = ObjectIntersection<T, Dim, ObjType>(objectIntersection, obj);
            statistic::stat.spatial_structure_no_nearer_intersection_tests++;
            assert(best.hit && best.object != nullptr);
			if (stopOnFirstIntersection) return best;
        }
    }
    
    return best;
}

// Explicit template instantiation for common types to ensure linkage
template class BruteForce<float, 3, Sphere<float, 3>>;
template class BruteForce<float, 3, Triangle<float, 3>>;
template class BruteForce<float, 3, TriangleWithNormals<float, 3>>;

} // END namespace geom
