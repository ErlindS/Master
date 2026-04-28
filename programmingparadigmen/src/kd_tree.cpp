#include "kd_tree.h"
#include <iostream>
#include <stack>
#include <limits>
#include "statistics.h"

template<typename T, size_t Dim, typename ObjType>
AABB<T, Dim> calculateAABB(const std::vector<std::shared_ptr<ObjType>>& objects) {
    if (objects.empty()) {
        // Handle case where there are no triangles in the vector
        return AABB<T, Dim>(Vector<T, Dim>(0), Vector<T, Dim>(0));
    }

    // Initialize min and max to the first triangle's vertices
    AABB<T, Dim> firstAABB = objects[0]->getAABB();
    Vector<T, Dim> min = firstAABB.min;
    Vector<T, Dim> max = firstAABB.max;

    // Now iterate through the rest of the triangles, expanding min and max as needed
    for (size_t i = 1; i < objects.size(); ++i) {
        AABB<T, Dim> currentAABB = objects[i]->getAABB();
        for (size_t j = 0; j < Dim; ++j) {
            min[j] = std::min(min[j], currentAABB.min[j]);
            max[j] = std::max(max[j], currentAABB.max[j]);
        }
    }

    return AABB<T, Dim>(min, max);
}

template<typename F, size_t D, typename O>
std::ostream& operator<<(std::ostream& os, const KDTree<F, D, O>& tree) {
    os << "KD-Tree Structure:" << std::endl;
    tree.printTree(os, tree.root.get(), 0);
    return os;
}

template<typename T, size_t Dim, typename ObjType>
void KDTree<T, Dim, ObjType>::printTree(std::ostream& os, const Node* node, size_t depth) const {
    if (node == nullptr) {
        return;
    }

    // Print indentation for current depth
    for (size_t i = 0; i < depth; i++) {
        os << "  ";
    }

    // Print node information
    os << "Node at depth " << depth << std::endl;
    os << "  Split Axis: " << node->split_axis << std::endl;
    os << "  Split Point: ";
    for (size_t i = 0; i < Dim; i++) {
        os << node->split_point[i] << " ";
    }
    os << std::endl;
    os << "  Objects: " << node->objects.size() << std::endl;
    for (const auto& obj : node->objects) {
        os << "    Object: " << obj << std::endl;
    }

    // Recursively print left and right child nodes
    if (node->left != nullptr) {
        os << "  Left Child:" << std::endl;
        printTree(os, node->left.get(), depth + 1);
    }

    if (node->right != nullptr) {
        os << "  Right Child:" << std::endl;
        printTree(os, node->right.get(), depth + 1);
    }
}
template<typename T, size_t Dim, typename ObjType>
void KDTree<T, Dim, ObjType>::buildTree(std::vector<std::shared_ptr<ObjType>>& objects, size_t depth, std::unique_ptr<Node>& node) {
	node->aabb = calculateAABB<T, Dim, ObjType>(objects);
	// no kd tree implemented yet
	node->objects = objects;
	node->aabb_nodes = node->aabb;
}

template<typename T, size_t Dim, typename ObjType>
void KDTree<T, Dim, ObjType>::build(std::vector<std::shared_ptr<ObjType>>& objects, const std::string& strategy) {
    all_objects = objects; 
    root = std::make_unique<Node>();
    buildTree(all_objects, 0, root);
}
 

template<typename T, size_t Dim, typename ObjType>
ObjectIntersection<T, Dim, ObjType> KDTree<T, Dim, ObjType>::nearestIntersection(const Ray<T, Dim>& ray, const Node* node, ObjectIntersection<T, Dim, ObjType> best, bool stopOnFirstIntersection,  int depth)  {
    // no kd tree implemented yet
    best = SpatialStructure<T, Dim, ObjType>::bruteForceNearestIntersection(ray, node->objects, best, stopOnFirstIntersection);
 
    return best;
}

template<typename T, size_t Dim, typename ObjType>
ObjectIntersection<T, Dim, ObjType> KDTree<T, Dim, ObjType>::nearestIntersection(const Ray<T, Dim>& ray, ObjectIntersection<T, Dim, ObjType> best, bool stopOnFirstIntersection,  int depth)  {
	  return nearestIntersection(ray, root.get(), best, stopOnFirstIntersection, depth);
}

