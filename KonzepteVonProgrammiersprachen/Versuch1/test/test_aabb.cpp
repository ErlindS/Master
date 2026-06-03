#include <gtest/gtest.h>
#include "kdtree.h"
#include "ray.h"
#include "test_helpers.h"

TEST(AABBTest, HitCenter) {
    AABB b = unit_box();
    Ray r(glm::vec3(0,0,5), glm::vec3(0,0,-1));
    float tMin=0, tMax=100;
    EXPECT_TRUE(b.intersect(r, tMin, tMax));
}

TEST(AABBTest, MissSide) {
    AABB b = unit_box();
    Ray r(glm::vec3(5,0,5), glm::vec3(0,0,-1));
    float tMin=0, tMax=100;
    EXPECT_FALSE(b.intersect(r, tMin, tMax));
}

TEST(AABBTest, RayBehindBox) {
    AABB b = unit_box();
    Ray r(glm::vec3(0,0,5), glm::vec3(0,0,1));
    float tMin=0, tMax=100;
    EXPECT_FALSE(b.intersect(r, tMin, tMax));
}

TEST(AABBTest, TMaxCutsHit) {
    AABB b = unit_box();
    Ray r(glm::vec3(0,0,5), glm::vec3(0,0,-1));
    float tMin=0, tMax=3;
    EXPECT_FALSE(b.intersect(r, tMin, tMax)); 
}

TEST(AABBTest, EdgeHit) {
    AABB b = unit_box();
    Ray r(glm::vec3(1,1,5), glm::vec3(0,0,-1));
    float tMin=0, tMax=100;
    EXPECT_TRUE(b.intersect(r, tMin, tMax));
}

TEST(AABBTest, RayOriginInside) {
    AABB b = unit_box();
    Ray r(glm::vec3(0,0,0), glm::vec3(1,1,1));
    float tMin=0, tMax=100;
    EXPECT_TRUE(b.intersect(r, tMin, tMax));
}

TEST(AABBTest, FlatBoxHit) {
    AABB b; 
    b.min = glm::vec3(-1,-1, 0); 
    b.max = glm::vec3( 1, 1, 0); 
    Ray r(glm::vec3(0,0,5), glm::vec3(0,0,-1));
    float tMin=0, tMax=100;
    EXPECT_TRUE(b.intersect(r, tMin, tMax));
}

TEST(AABBTest, ExpandByPoint) {
    AABB b;
    b.expand(glm::vec3(1,2,3)); 
    b.expand(glm::vec3(-1,-2,-3));
    EXPECT_NEAR(b.min.x, -1, 1e-6f);
    EXPECT_NEAR(b.max.x,  1, 1e-6f);
    EXPECT_NEAR(b.min.z, -3, 1e-6f);
    EXPECT_NEAR(b.max.z,  3, 1e-6f);
}
