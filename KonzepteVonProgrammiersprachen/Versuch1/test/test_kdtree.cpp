#include <gtest/gtest.h>
#include "kdtree.h"
#include "ray.h"
#include "test_helpers.h"
#include <vector>

TEST(KDTreeTest, Empty) {
    KDTree kd;
    std::vector<Triangle> tris;
    kd.build(tris);
    Ray r; r.origin = {0,0,5}; r.direction = {0,0,-1};
    float t,u,v;
    EXPECT_EQ(kd.closest_hit(tris, r, 1e-4f, 1e30f, t, u, v), -1);
}

TEST(KDTreeTest, SingleHit) {
    std::vector<Triangle> tris = {xy_tri()};
    KDTree kd; kd.build(tris);
    Ray r; r.origin = {0.25f,0.25f,1.f}; r.direction = {0,0,-1};
    float t,u,v;
    int idx = kd.closest_hit(tris, r, 1e-4f, 1e30f, t, u, v);
    EXPECT_EQ(idx, 0);
    EXPECT_NEAR(t, 1.f, 1e-4f);
}

TEST(KDTreeTest, NearestOfTwo) {
    Triangle t1 = xy_tri();
    Triangle t2 = xy_tri();
    t2.v0.z = t2.v1.z = t2.v2.z = 2.f; 
    std::vector<Triangle> tris = {t1, t2};
    KDTree kd; kd.build(tris);
    
    Ray r; r.origin = {0.25f,0.25f,5.f}; r.direction = {0,0,-1};
    float t,u,v;
    int idx = kd.closest_hit(tris, r, 1e-4f, 1e30f, t, u, v);
    EXPECT_EQ(idx, 1);
    EXPECT_NEAR(t, 3.f, 1e-4f);
}

TEST(KDTreeTest, HitFarVsNearNode) {
    Triangle tNear = xy_tri(); tNear.v0.z = tNear.v1.z = tNear.v2.z = 1.0f; 
    Triangle tFar = xy_tri();  tFar.v0.z  = tFar.v1.z  = tFar.v2.z  = -1.0f; 
    std::vector<Triangle> tris = {tNear, tFar};
    KDTree kd; kd.build(tris);
    
    Ray r; r.origin = {0.2f,0.2f,5.f}; r.direction = {0,0,-1};
    float t,u,v;
    int idx = kd.closest_hit(tris, r, 1e-4f, 1e30f, t, u, v);
    
    EXPECT_EQ(idx, 0); 
}

TEST(KDTreeTest, AnyHitTrue) {
    std::vector<Triangle> tris = {xy_tri()};
    KDTree kd; kd.build(tris);
    Ray r; r.origin = {0.25f,0.25f,1.f}; r.direction = {0,0,-1};
    EXPECT_TRUE(kd.any_hit(tris, r, 1e-4f, 1e30f));
}

TEST(KDTreeTest, AnyHitBehindOrigin) {
    std::vector<Triangle> tris = {xy_tri()}; 
    KDTree kd; kd.build(tris);
    Ray r; r.origin = {0.25f,0.25f,1.f}; r.direction = {0,0,1}; 
    EXPECT_FALSE(kd.any_hit(tris, r, 1e-4f, 1e30f));
}
