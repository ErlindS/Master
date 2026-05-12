#ifndef KD_TREE_H
#define KD_TREE_H

#include <memory>
#include <vector>
#include <algorithm>
#include <limits>
#include "vector.h"
#include "geometry.h"
#include "spatial_structure.h"

using namespace math;
using namespace geom;

/**
 * @class KDTree
 * @brief KD-Tree implementation for spatial partitioning of geometric objects in n-dimensional space.
 *
 * This KD-Tree structure is designed to facilitate fast nearest neighbor searches and 
 * ray intersection tests by partitioning space into a binary tree where each node 
 * represents an axis-aligned split. Objects can be placed at nodes if they span 
 * across the split plane, which is particularly beneficial for large or oddly shaped objects.
 *
 * @tparam T The floating-point type used for vector components (e.g., float, double).
 * @tparam Dim The dimensionality of the space, typically 2 or 3 for spatial applications.
 * @tparam ObjType The type of geometric object stored in the tree, must have a method to intersect with rays.
 */
template<typename T, size_t Dim, typename ObjType>
class KDTree : public SpatialStructure<T, Dim, ObjType> {
private:
    /**
     * @struct Node
     * @brief Represents a node in the KD-Tree structure.
     *
     * Each node can split along one axis and may contain objects that span this split.
     */
    struct Node {
        std::unique_ptr<Node> left;  ///< Pointer to the left child node.
        std::unique_ptr<Node> right; ///< Pointer to the right child node.
        std::vector<std::shared_ptr<ObjType>> objects; ///< Objects at this node if they span the split plane and no further subdivision is useful.
        Vector<T, Dim> split_point;   ///< The point where this node splits the space.
        AABB<T, Dim> aabb;            ///< AABB of all objects stored at this node and childs
		AABB<T, Dim> aabb_nodes;      ///< AABB of all objects stored at this node without objects of subnodes
        size_t split_axis;            ///< The axis along which this node splits.

        Node() : left(nullptr), right(nullptr), split_axis(0) {}
    };

    std::unique_ptr<Node> root; ///< The root node of the KD-Tree.
    std::vector<std::shared_ptr<ObjType>> all_objects; ///< All objects used for constructing the tree.

    /* Cache-freundlichere Alternative ohne shared_ptr
	struct Node {
      int left;    // Index des linken Kindknotens, -1 falls nicht vorhanden
      int right;   // Index des rechten Kindknotens, -1 falls nicht vorhanden
      size_t obj_start; // Startindex im zentralen Objekte-Array
      size_t obj_end;   // Endindex (exklusiv) im zentralen Objekte-Array
      // ... weitere Felder wie split_axis, split_point, aabb etc.
     };
     
	 std::vector<Node> nodes;      // die Nodes sollte möglichst in Reihenfolge der Traversierung gespeichert sein
     std::vector<ObjType> objects; // die Objekte eines Node müssen sequentiell in diesem vector gespeichert sein!
    */
	
    /**
     * @brief Recursively builds the KD-Tree from a list of objects.
     * 
     * @param objects Vector of shared pointers to geometric objects.
     * @param depth Current depth in the tree, used to determine the split axis.
     * @param node Pointer to the current node being processed or created.
     */
    void buildTree(std::vector<std::shared_ptr<ObjType>>& objects, size_t depth, std::unique_ptr<Node>& node);

     ObjectIntersection<T, Dim, ObjType>nearestIntersection(const Ray<T, Dim>& ray, const Node* node, ObjectIntersection<T, Dim, ObjType> best, bool stopOnFirstIntersection = false,  int depth = 0) ;

    /**
     * @brief Recursively finds the nearest intersection with a given ray.
     * 
     * @param ray The ray to test for intersection.
     * @param best Reference to the best intersection found so far.
	 * @param stopOnFirstIntersection Ff true the first intersection is returned, not necessarly the nearest. Useful for instance for shadow rays.
     * @return Updated intersection information.
     */
    ObjectIntersection<T, Dim, ObjType> nearestIntersection(const Ray<T, Dim>& ray, ObjectIntersection<T, Dim, ObjType> best, bool stopOnFirstIntersection = false, int depth = 0) override;
public:
    /**
     * @brief Default constructor for KDTree.
     */
    KDTree() = default;
	
    /**
     * @brief Destructor for KDTree, ensuring all allocated memory is properly released.
     *
     * By using smart pointers, we ensure automatic destruction and thus no need for an explicit destructor.
     */
    ~KDTree() = default;
	
    /**
     * @brief Builds the KD-Tree from a list of geometric objects using a specified strategy.
     * 
     * @param objects Vector of shared pointers to geometric objects to be inserted into the tree.
     * @param strategy String specifying the tree construction method ("median" or "sah").
     */
    void build(std::vector<std::shared_ptr<ObjType>> & objects, const std::string& strategy = "median") override;


    // Delete copy constructor and assignment operator to prevent copying of KDTree
    /**
     * @brief Copy constructor is deleted to prevent accidental copying of KDTree.
     */
    KDTree(const KDTree&) = delete;

    /**
     * @brief Assignment operator is deleted to prevent assignment of KDTree.
     */
    KDTree& operator=(const KDTree&) = delete;
	
    void printTree(std::ostream& os, const Node* node, size_t depth) const;
 
    template<typename F, size_t D, typename O>
    friend std::ostream& operator<<(std::ostream& os, const KDTree<F, D, O>& tree);
};

// Typedefs for common types
typedef KDTree<float, 3, Triangle<float, 3>> KDTree3fTriangle;  ///< KD-Tree for 3D triangles using float precision.
typedef KDTree<float, 3, Sphere<float, 3>> KDTree3fSphere;     ///< KD-Tree for 3D spheres using float precision.

#endif // KD_TREE_H
