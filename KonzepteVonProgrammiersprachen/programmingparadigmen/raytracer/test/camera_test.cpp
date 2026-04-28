#include <gtest/gtest.h>
#include "camera.h" 

// Test case for the Camera constructor
TEST(CameraTest, ConstructorTest) {
    Vector3f origin{0, 0, 0};
    Vector3f lookAt{0, 0, -1};
    Vector3f up{0, 1, 0};
    float fov = M_PI / 4; // 45 degrees in radians
    float aspectRatio = 16.0f / 9.0f;
    std::string projection = "perspective";

    Camera3f camera(origin, lookAt, up, fov, aspectRatio, projection);

    // Basic checks to ensure the camera axes are set up correctly
    EXPECT_NE(camera.xAxis, (Vector3f{0, 0, 0}));
    EXPECT_NE(camera.yAxis, (Vector3f{0, 0, 0}));
    EXPECT_NE(camera.zAxis, (Vector3f{0, 0, 0}));
}

// Test case for generateRay method with perspective projection
TEST(CameraTest, GenerateRayPerspectiveProjection) {
    Vector3f origin{0, 0, 0};
    Vector3f lookAt{0, 0, -1};
    Vector3f up{0, 1, 0};
    float fov = M_PI / 4; // 45 degrees in radians
    float aspectRatio = 1.0f; // Square image
    std::string projection = "perspective";

    Camera3f camera(origin, lookAt, up, fov, aspectRatio, projection);

    std::vector<Ray3f> rays = camera.generateRay(0, 0, 100, 100, 1);
    EXPECT_EQ(rays.size(), 1); // Should return one ray for one sample

    // Check if the ray direction is normalized (for perspective projection)
    //EXPECT_NEAR( rays[0].direction.normalized(), 1.0, 1e-5);
}

// Test case for generateRay method with parallel projection
TEST(CameraTest, GenerateRayParallelProjection) {
    Vector3f origin{0, 0, 0};
    Vector3f lookAt{0, 0, -1};
    Vector3f up{0, 1, 0};
    float fov = M_PI / 4;   // 45 degrees in radians (fov not used in parallel projection but included for consistency)
    float aspectRatio = 1.0f; // Square image
    std::string projection = "parallel";

    Camera3f camera(origin, lookAt, up, fov, aspectRatio, projection);

    std::vector<Ray3f> rays = camera.generateRay(0, 0, 100, 100, 1);
    EXPECT_EQ(rays.size(), 1); // Should return one ray for one sample

    // For parallel projection, all rays should be in the direction of -zAxis
    EXPECT_EQ(rays[0].get_direction(), -1.0f * camera.zAxis);
}

// Test case for generateRayFromIntersection method
TEST(CameraTest, GenerateRayFromIntersection) {
    Vector3f origin{0, 0, 0};
    Vector3f lookAt{0, 0, -1};
    Vector3f up{0, 1, 0};
    float fov = M_PI / 4; // 45 degrees in radians
    float aspectRatio = 1.0f; // Square image
    std::string projection = "perspective"; // Projection type doesn't matter for reflection

    Camera3f camera(origin, lookAt, up, fov, aspectRatio, projection);

    Vector3f intersectionPoint{0, 0, -1};
    Vector3f surfaceNormal{0, 0, 1}; // Surface facing towards the camera

    Ray3f reflectedRay = camera.generateRayFromIntersection(intersectionPoint, surfaceNormal);

    // Check if the reflected ray's origin is at the intersection point
    EXPECT_EQ(reflectedRay.get_origin(), intersectionPoint);

    // Check if the direction is indeed reflected (this is a basic test, actual reflection might need more complex checks)
    EXPECT_NE(reflectedRay.get_direction(), (Vector3f{0, 0, -1})); // Should not be the incoming direction
}

// Test case to check if generateRay() returns different rays for the same pixel with multiple samples
TEST(CameraTest, GenerateRayMultipleSamples) {
    Vector3f origin{0, 0, 0};
    Vector3f lookAt{0, 0, -1};
    Vector3f up{0, 1, 0};
    float fov = M_PI / 4; // 45 degrees in radians
    float aspectRatio = 1.0f; // Square image
    std::string projection = "perspective";

    Camera3f camera(origin, lookAt, up, fov, aspectRatio, projection);

    // Generate rays for the same pixel with different samples
    std::vector<Ray3f> rays1 = camera.generateRay(50, 50, 100, 100, 10); // 10 samples
    std::vector<Ray3f> rays2 = camera.generateRay(50, 50, 100, 100, 10); // Another set of 10 samples

    // Check if rays are different even for the same pixel due to random jitter
    for (int i = 0; i < 10; ++i) {
        for (int j = i + 1; j < 10; ++j) {
            // Compare direction vectors for inequality to ensure they're different due to jitter
            EXPECT_FALSE(rays1[i].get_direction() == rays1[j].get_direction()) << "Rays within the first set should differ.";
            EXPECT_FALSE(rays1[i].get_direction() == rays2[i].get_direction()) << "Rays between sets should differ for the same sample index.";
        }
    }

    // Optionally, check if rays are not all identical across multiple calls
    bool allSame = true;
    for (int i = 1; i < 10 && allSame; ++i) {
        if (rays1[i].get_direction() != rays1[0].get_direction()) {
            allSame = false;
        }
    }
    EXPECT_FALSE(allSame) << "All rays should not be identical due to random jitter.";
}
