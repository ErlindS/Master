#include "kd_tree.h"
#include "kd_tree.cpp"

template class KDTree<float, 3, Sphere<float, 3>>;
template class KDTree<float, 3, Triangle<float, 3>>;
template class KDTree<float, 3, TriangleWithNormals<float, 3>>;
template class ObjectIntersection<float, 3, Sphere<float, 3>>;
template class ObjectIntersection<float, 3, Triangle<float, 3>>;
template class ObjectIntersection<float, 3, TriangleWithNormals<float, 3>>;
template std::ostream& operator<<(std::ostream& os, const KDTree<float, 3, Sphere<float, 3> > & tree);
template std::ostream& operator<<(std::ostream& os, const KDTree<float, 3, Triangle<float, 3> > & tree);
template std::ostream& operator<<(std::ostream& os, const KDTree<float, 3, TriangleWithNormals<float, 3> > & tree);