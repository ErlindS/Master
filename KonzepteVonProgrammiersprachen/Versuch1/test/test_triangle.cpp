#include <gtest/gtest.h>
#include "mesh.h"
#include "intersector.h"
#include "ray.h"
#include "test_helpers.h"

TEST(TriangleTest, MTHit) {
    Triangle tri = xy_tri();
    Ray r(glm::vec3(0.25f,0.25f,1.f), glm::vec3(0,0,-1));
    Intersection isect = Intersector::intersectRayTriangle(r, tri.v0, tri.v1, tri.v2);
    EXPECT_TRUE(isect.hit);
    EXPECT_NEAR(isect.t,  1.f,   1e-5f);
    EXPECT_NEAR(isect.u,  0.25f, 1e-5f);
    EXPECT_NEAR(isect.v,  0.25f, 1e-5f);
}

TEST(TriangleTest, MTMissOutside) {
    Triangle tri = xy_tri();
    Ray r(glm::vec3(0.8f,0.8f,1.f), glm::vec3(0,0,-1));
    Intersection isect = Intersector::intersectRayTriangle(r, tri.v0, tri.v1, tri.v2);
    EXPECT_FALSE(isect.hit);
}

TEST(TriangleTest, MTBackfaceHit) {
    Triangle tri = xy_tri();
    Ray r(glm::vec3(0.25f,0.25f,-1.f), glm::vec3(0,0,1));
    Intersection isect = Intersector::intersectRayTriangle(r, tri.v0, tri.v1, tri.v2);
    EXPECT_TRUE(isect.hit);
    EXPECT_NEAR(isect.t,  1.f,   1e-5f);
}

TEST(TriangleTest, BadouelAgreesWithMT) {
    Triangle tri = xy_tri();
    struct TC { float ox,oy,oz; } cases[] = {
        {0.1f, 0.1f, 2.f}, {0.5f, 0.4f, 1.f}, {0.9f, 0.05f, 1.f},
        {2.0f, 0.f,  1.f}, 
    };
    for (auto& c : cases) {
        Ray r(glm::vec3(c.ox,c.oy,c.oz), glm::vec3(0,0,-1));
        Intersection isect1 = Intersector::intersectRayTriangle(r, tri.v0, tri.v1, tri.v2);
        Intersection isect2 = Intersector::intersectRayTriangleBadouel(r, tri.v0, tri.v1, tri.v2);
        EXPECT_EQ(isect1.hit, isect2.hit);
        if (isect1.hit && isect2.hit) {
            EXPECT_NEAR(isect1.t, isect2.t, 1e-4f);
            EXPECT_NEAR(isect1.u, isect2.u, 1e-4f);
            EXPECT_NEAR(isect1.v, isect2.v, 1e-4f);
        }
    }
}
