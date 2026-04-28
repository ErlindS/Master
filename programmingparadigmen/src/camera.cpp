#include "camera.h"

template<typename T>
Camera<T>::Camera(Vector<T, 3> origin, Vector<T, 3> lookAt, Vector<T, 3> up, T fov, T aspectRatio, std::string projection)
    : origin(origin), lookAt(lookAt), up(up), fov(fov), aspectRatio(aspectRatio), projection(projection)
{
    // Ensure the field of view is within reasonable limits
    assert(fov > 0 && fov < M_PI); // fov should be between 0 and π radians
    assert(aspectRatio > 0); // aspectRatio must be positive

    // Calculate the z-axis (view direction) as the vector from the camera to the target point and normalize it
    this->zAxis = (origin - lookAt).normalized();
    // Calculate the x-axis (right side) by crossing up with z-axis, ensuring they are orthogonal
    this->xAxis = up.cross(zAxis).normalized();
    // Calculate the y-axis (up) by crossing z-axis with x-axis to form a right-handed coordinate system
    this->yAxis = zAxis.cross(xAxis);
}

template<typename T>
std::vector<Ray<T, 3>> Camera<T>::generateRay(int x, int y, int width, int height, int numSamples) const
{
    std::vector<Ray<T, 3>> rays;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<T> dis(0.0, 1.0);

    // Calculate scaling based on the field of view (fov) to determine the image plane
    T scale = std::tan(fov / 2.0);
    T imageAspectRatio = aspectRatio;
    
    // Check for valid input dimensions
    assert(width > 0 && height > 0); // Width and height should be positive
    assert(numSamples > 0); // Number of samples should be positive

    for (int s = 0; s < numSamples; ++s)
    {
        // Jitter for antialiasing: add small random offsets to pixel coordinates
        T px = (2 * (x + dis(gen)) / (width - 1) - 1) * scale * imageAspectRatio;
        T py = (1 - 2 * (y + dis(gen)) / (height - 1)) * scale;

        Vector<T, 3> rayDirection;

        if (projection == "perspective")
        {
            // Calculate ray direction for perspective projection where rays pass through the image point from the camera
            rayDirection = (xAxis * px + yAxis * py - zAxis).normalized();
        }
        else if (projection == "parallel")
        {
            // For orthographic (parallel) projection where all rays run parallel to the view direction
            rayDirection = T(-1.0) * zAxis; // Direction is simply negative z-axis
        }
        else
        {
            // Error handling for unknown projection type
            std::cerr << "Unknown projection type: " << projection << std::endl;
            continue; // or some default projection
        }

        rays.push_back(Ray<T, 3>(origin, rayDirection));
    }

    return rays;
}

template<typename T>
Ray<T, 3> Camera<T>::generateRayFromIntersection(Vector<T, 3> intersectionPoint, Vector<T, 3> surfaceNormal)
{
    // Calculate the reflection direction of a ray at an intersection point based on the surface normal
    // This method uses the reflection formula: r = d - 2 * (d . n) * n, where d is the direction from camera to intersection
    Vector<T, 3> reflectionDirection = surfaceNormal * (-2.0 * surfaceNormal.dot(intersectionPoint - origin)) + (intersectionPoint - origin);

    return Ray<T, 3>(intersectionPoint, reflectionDirection);
}

template<typename T>
T Camera<T>::getFov() const {
	return this->fov;
}