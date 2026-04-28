#ifndef CAMERA_H
#define CAMERA_H

#include <random>
#include <cmath>
#include <iostream>
#include "vector.h"
#include "geometry.h"

using namespace math;
using namespace geom;
/**
 * @brief The Camera class represents a camera in a 3D space, useful for rendering and ray tracing applications.
 * 
 * This class provides functionality to manage camera parameters like position, orientation, field of view,
 * aspect ratio, and projection type. It supports both perspective and orthographic (parallel) projections.
 *
 * @tparam T The data type used for coordinate calculations, typically float or double for precision.
 */
template<typename T>
class Camera {
public:
    /**
     * @brief The origin or position of the camera in 3D space.
     */
    Vector<T, 3> origin;

    /**
     * @brief The point in 3D space that the camera is directed towards.
     */
    Vector<T, 3> lookAt;

    /**
     * @brief The up vector of the camera, used to orient the camera's coordinate system.
     */
    Vector<T, 3> up;

    /**
     * @brief The field of view angle of the camera in radians, determining the extent of the observable world.
     */
    T fov;

    /**
     * @brief The aspect ratio of the camera view, typically width/height of the image or viewport.
     */
    T aspectRatio;

    /**
     * @brief The projection type of the camera, defining how points are mapped onto the image plane.
     * Can be either "perspective" for a realistic 3D effect or "parallel" for orthographic views.
     */
    std::string projection;

    /**
     * @brief The local x-axis of the camera, pointing right in the camera's coordinate system.
     */
    Vector<T, 3> xAxis;

    /**
     * @brief The local y-axis of the camera, pointing upwards in the camera's coordinate system.
     */
    Vector<T, 3> yAxis;

    /**
     * @brief The local z-axis of the camera, pointing in the opposite direction of the lookAt vector.
     */
    Vector<T, 3> zAxis;

public:
    /**
     * @brief Constructor for the Camera class.
     *
     * @param origin The starting position of the camera in 3D space.
     * @param lookAt The focal point or direction vector that the camera looks at.
     * @param up The vector defining what is considered 'up' for this camera to orient its local coordinate system.
     * @param fov The field of view in radians, which affects how much of the scene is visible.
     * @param aspectRatio The aspect ratio of the camera's view, influencing the shape of the projection.
     * @param projection The type of projection, either "perspective" for depth perception or "parallel" for undistorted views.
     */
    Camera(Vector<T, 3> origin, Vector<T, 3> lookAt, Vector<T, 3> up, T fov, T aspectRatio, std::string projection);

    /**
     * @brief Generates multiple rays for a single pixel, useful for anti-aliasing and sampling.
     * 
     * This method computes ray directions based on pixel coordinates, incorporating jitter for smoother images.
     * 
     * @param x The x-coordinate of the pixel on the image plane.
     * @param y The y-coordinate of the pixel on the image plane.
     * @param width The width of the image in pixels.
     * @param height The height of the image in pixels.
     * @param numSamples Number of rays to generate for anti-aliasing or other sampling techniques.
     * @return std::vector<Ray<T, 3>> A vector of rays originating from the camera's origin.
     */
    std::vector<Ray<T, 3>> generateRay(int x, int y, int width, int height, int numSamples) const;

    /**
     * @brief Generates a single ray based on an intersection point and surface normal, typically used for reflections.
     * 
     * This function computes the direction of a reflected ray from the intersection point using the surface normal.
     * 
     * @param intersectionPoint The point where the previous ray hit the surface.
     * @param surfaceNormal The normal vector at the intersection point, indicating the orientation of the surface.
     * @return Ray<T, 3> A new ray representing the reflected light path.
     */
    Ray<T, 3> generateRayFromIntersection(Vector<T, 3> intersectionPoint, Vector<T, 3> surfaceNormal);
	
	T getFov() const;
	
};

// Typedef for convenience, using float for 3D vectors
typedef Camera<float> Camera3f;

#endif // CAMERA_H