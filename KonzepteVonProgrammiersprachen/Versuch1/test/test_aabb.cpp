#include <gtest/gtest.h>
#include "aabb.h"
#include "ray.h"
#include "test_helpers.h"

TEST(AABBTest, HitCenter) {
    AABB b = unit_box();
    Ray r; r.origin = {0,0,5}; r.direction = {0,0,-1};
    EXPECT_TRUE(b.intersect(r, 0, 100));
}

TEST(AABBTest, MissSide) {
    AABB b = unit_box();
    Ray r; r.origin = {5,0,5}; r.direction = {0,0,-1};
    EXPECT_FALSE(b.intersect(r, 0, 100));
}

TEST(AABBTest, RayBehindBox) {
    AABB b = unit_box();
    Ray r; r.origin = {0,0,5}; r.direction = {0,0,1};
    EXPECT_FALSE(b.intersect(r, 0, 100));
}

TEST(AABBTest, TMaxCutsHit) {
    AABB b = unit_box();
    Ray r; r.origin = {0,0,5}; r.direction = {0,0,-1};
    EXPECT_FALSE(b.intersect(r, 0, 3)); 
}

TEST(AABBTest, EdgeHit) {
    AABB b = unit_box();
    Ray r; r.origin = {1,1,5}; r.direction = {0,0,-1};
    EXPECT_TRUE(b.intersect(r, 0, 100));
}

TEST(AABBTest, RayOriginInside) {
    AABB b = unit_box();
    Ray r; r.origin = {0,0,0}; r.direction = {1,1,1};
    EXPECT_TRUE(b.intersect(r, 0, 100));
}

TEST(AABBTest, FlatBoxHit) {
    AABB b; 
    b.min = {-1,-1, 0}; 
    b.max = { 1, 1, 0}; 
    Ray r; r.origin = {0,0,5}; r.direction = {0,0,-1};
    EXPECT_TRUE(b.intersect(r, 0, 100));
}

TEST(AABBTest, ExpandByPoint) {
    AABB b;
    b.expand({1,2,3}); 
    b.expand({-1,-2,-3});
    EXPECT_NEAR(b.min.x, -1, 1e-6f);
    EXPECT_NEAR(b.max.x,  1, 1e-6f);
    EXPECT_NEAR(b.min.z, -3, 1e-6f);
    EXPECT_NEAR(b.max.z,  3, 1e-6f);
}
