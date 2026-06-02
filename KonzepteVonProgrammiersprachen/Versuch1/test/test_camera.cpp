#include <gtest/gtest.h>
#include "camera.h"
#include "ray.h"

TEST(CameraTest, CenterRay) {
    Camera cam({0,0,5},{0,0,0},{0,1,0}, 90.f, 1.f);
    Ray r = cam.get_ray(0.5f, 0.5f);
    EXPECT_NEAR(r.direction.x,  0, 1e-5f);
    EXPECT_NEAR(r.direction.y,  0, 1e-5f);
    EXPECT_NEAR(r.direction.z, -1, 1e-5f);
}

TEST(CameraTest, RayNormalized) {
    Camera cam({0,0,5},{0,0,0},{0,1,0}, 45.f, 16.f/9.f);
    float ss[] = {0.f, 0.25f, 0.75f, 1.f};
    float ts[] = {0.f, 0.5f,  1.f};
    for (float s : ss) {
        for (float t : ts) {
            EXPECT_NEAR(cam.get_ray(s,t).direction.length(), 1, 1e-5f);
        }
    }
}
