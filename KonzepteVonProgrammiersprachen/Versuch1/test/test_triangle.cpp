#include <gtest/gtest.h>
#include "triangle.h"
#include "ray.h"
#include "test_helpers.h"

TEST(TriangleTest, MTHit) {
    Triangle tri = xy_tri();
    Ray r; r.origin = {0.25f,0.25f,1.f}; r.direction = {0,0,-1};
    float u, v, t = tri.intersect(r, 0, 10, u, v);
    EXPECT_NEAR(t,  1.f,   1e-5f);
    EXPECT_NEAR(u,  0.25f, 1e-5f);
    EXPECT_NEAR(v,  0.25f, 1e-5f);
}

TEST(TriangleTest, MTMissOutside) {
    Triangle tri = xy_tri();
    Ray r; r.origin = {0.8f,0.8f,1.f}; r.direction = {0,0,-1};
    float u,v; float t = tri.intersect(r, 0, 10, u, v);
    EXPECT_TRUE(t < 0);
}

TEST(TriangleTest, MTBackfaceHit) {
    Triangle tri = xy_tri();
    Ray r; r.origin = {0.25f,0.25f,-1.f}; r.direction = {0,0,1};
    float u,v; float t = tri.intersect(r, 0, 10, u, v);
    EXPECT_NEAR(t,  1.f,   1e-5f);
}

TEST(TriangleTest, BadouelAgreesWithMT) {
    Triangle tri = xy_tri();
    struct TC { float ox,oy,oz; } cases[] = {
        {0.1f, 0.1f, 2.f}, {0.5f, 0.4f, 1.f}, {0.9f, 0.05f, 1.f},
        {2.0f, 0.f,  1.f}, 
    };
    for (auto& c : cases) {
        Ray r; r.origin = {c.ox,c.oy,c.oz}; r.direction = {0,0,-1};
        float u1,v1,u2,v2;
        float t1 = tri.intersect(r, 0, 100, u1, v1);
        float t2 = tri.intersect_naive(r, 0, 100, u2, v2);
        EXPECT_EQ((t1 > 0), (t2 > 0));
        if (t1 > 0 && t2 > 0) {
            EXPECT_NEAR(t1, t2, 1e-4f);
            EXPECT_NEAR(u1, u2, 1e-4f);
            EXPECT_NEAR(v1, v2, 1e-4f);
        }
    }
}
