#include <gtest/gtest.h>
#include "kdtree.h"
#include "ray.h"
#include "test_helpers.h"
#include <vector>

TEST(KDTreeTest, Empty) {
    KDTree kd;
    std::vector<Triangle> tris;
    kd.build(tris);
    Ray r(glm::vec3(0,0,5), glm::vec3(0,0,-1));
    Intersection isect; isect.t = 1e30f; isect.hit = false;
    Triangle hitTri;
    EXPECT_FALSE(kd.intersect(r, isect, hitTri, IntersectionAlgorithm::MOELLER_TRUMBORE));
}

TEST(KDTreeTest, SingleHit) {
    std::vector<Triangle> tris = {xy_tri()};
    KDTree kd; kd.build(tris);
    Ray r(glm::vec3(0.25f,0.25f,1.f), glm::vec3(0,0,-1));
    Intersection isect; isect.t = 1e30f; isect.hit = false;
    Triangle hitTri;
    EXPECT_TRUE(kd.intersect(r, isect, hitTri, IntersectionAlgorithm::MOELLER_TRUMBORE));
    EXPECT_NEAR(isect.t, 1.f, 1e-4f);
}

TEST(KDTreeTest, NearestOfTwo) {
    Triangle t1 = xy_tri();
    Triangle t2 = xy_tri();
    t2.v0.z = t2.v1.z = t2.v2.z = 2.f; 
    std::vector<Triangle> tris = {t1, t2};
    KDTree kd; kd.build(tris);
    
    Ray r(glm::vec3(0.25f,0.25f,5.f), glm::vec3(0,0,-1));
    Intersection isect; isect.t = 1e30f; isect.hit = false;
    Triangle hitTri;
    EXPECT_TRUE(kd.intersect(r, isect, hitTri, IntersectionAlgorithm::MOELLER_TRUMBORE));
    EXPECT_NEAR(isect.t, 3.f, 1e-4f);
}

TEST(KDTreeTest, HitFarVsNearNode) {
    Triangle tNear = xy_tri(); tNear.v0.z = tNear.v1.z = tNear.v2.z = 1.0f; 
    Triangle tFar = xy_tri();  tFar.v0.z  = tFar.v1.z  = tFar.v2.z  = -1.0f; 
    std::vector<Triangle> tris = {tNear, tFar};
    KDTree kd; kd.build(tris);
    
    Ray r(glm::vec3(0.2f,0.2f,5.f), glm::vec3(0,0,-1));
    Intersection isect; isect.t = 1e30f; isect.hit = false;
    Triangle hitTri;
    EXPECT_TRUE(kd.intersect(r, isect, hitTri, IntersectionAlgorithm::MOELLER_TRUMBORE));
    EXPECT_NEAR(isect.t, 4.f, 1e-4f);
}
