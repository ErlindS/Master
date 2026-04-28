#include "geometry.h"
#include "geometry.cpp"

namespace geom {

// Explicit template instantiations for float and dimensions 2, 3
template Intersection<float, 2> AABB<float, 2>::intersects(const Ray<float, 2>&, float) const;
template Intersection<float, 3> AABB<float, 3>::intersects(const Ray<float, 3>&, float) const;
template float AABB<float, 2>::getSize() const;
template float AABB<float, 3>::getSize() const;
template bool AABB<float, 2>::all_less_than_or_equal(const Vector<float, 2>&, const Vector<float, 2>&);
template bool AABB<float, 3>::all_less_than_or_equal(const Vector<float, 3>&, const Vector<float, 3>&);
template Intersection<float, 2> Sphere<float, 2>::intersect(const Ray<float, 2>&, float) const;
template Intersection<float, 3> Sphere<float, 3>::intersect(const Ray<float, 3>&, float) const;
template Intersection<float, 2> Triangle<float, 2>::intersect(const Ray<float, 2>&, float ) const;
template Intersection<float, 3> Triangle<float, 3>::intersect(const Ray<float, 3>&, float ) const;
template Intersection<float, 2> TriangleWithNormals<float, 2>::intersect(const Ray<float, 2>&, float ) const;
template Intersection<float, 3> TriangleWithNormals<float, 3>::intersect(const Ray<float, 3>&, float ) const;
template Vector<float, 2> Intersection<float, 2>::getIntersectionPoint(Ray<float, 2> const&) const;
template Vector<float, 3> Intersection<float, 3>::getIntersectionPoint(Ray<float, 3> const&) const;
template Vector<float, 2> Sphere<float, 2>::getCentroid() const;
template Vector<float, 3> Sphere<float, 3>::getCentroid() const;
template Vector<float, 2> Triangle<float, 2>::getCentroid() const;
template Vector<float, 3> Triangle<float, 3>::getCentroid() const;
template AABB<float, 2> Sphere<float, 2>::getAABB() const;
template AABB<float, 3> Sphere<float, 3>::getAABB() const;
template AABB<float, 2> Triangle<float, 2>::getAABB() const;
template AABB<float, 3> Triangle<float, 3>::getAABB() const;
template Vector<float, 2> TriangleWithNormals<float, 2>::getCentroid() const;
template Vector<float, 3> TriangleWithNormals<float, 3>::getCentroid() const;

template std::ostream& operator<<(std::ostream& os, const Sphere<float, 3>&);



template std::vector<Triangle<float, 3> > create_prisma<float>(
    const Vector<float, 3>, 
    const Vector<float, 3>, 
    const Vector<float, 3>, 
    const Vector<float, 3>,  
    bool, 
    const std::shared_ptr<BaseMaterial>&);



template class Ray<float, 2>;
template class Ray<float, 3>;
template struct Intersection<float, 2>;
template struct Intersection<float, 3>;
template struct Sphere<float, 2>;
template struct Sphere<float, 3>;
template class Triangle<float,2>;
template class Triangle<float,3>;
template class TriangleWithNormals<float,2>;
template class TriangleWithNormals<float,3>;
template class TriangleIntersectionPolicy<float, 2>;
template class TriangleIntersectionPolicy<float, 3>;

}