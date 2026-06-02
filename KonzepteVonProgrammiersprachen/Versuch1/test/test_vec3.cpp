#include <gtest/gtest.h>
#include "vec3.h"

TEST(Vec3Test, Add) {
    Vec3 c = Vec3{1,2,3} + Vec3{4,5,6};
    EXPECT_NEAR(c.x, 5, 1e-6f);
    EXPECT_NEAR(c.y, 7, 1e-6f);
    EXPECT_NEAR(c.z, 9, 1e-6f);
}

TEST(Vec3Test, Sub) {
    Vec3 c = Vec3{3,3,3} - Vec3{1,2,3};
    EXPECT_NEAR(c.x, 2, 1e-6f);
    EXPECT_NEAR(c.y, 1, 1e-6f);
    EXPECT_NEAR(c.z, 0, 1e-6f);
}

TEST(Vec3Test, Scale) {
    Vec3 b = Vec3{1,2,3} * 2.f;
    EXPECT_NEAR(b.x, 2, 1e-6f);
    EXPECT_NEAR(b.y, 4, 1e-6f);
    EXPECT_NEAR(b.z, 6, 1e-6f);
}

TEST(Vec3Test, Dot) {
    EXPECT_NEAR(Vec3(1,0,0).dot({0,1,0}), 0,  1e-6f);
    EXPECT_NEAR(Vec3(1,0,0).dot({1,0,0}), 1,  1e-6f);
    EXPECT_NEAR(Vec3(1,2,3).dot({4,5,6}), 32, 1e-6f);
}

TEST(Vec3Test, Cross) {
    Vec3 z = Vec3{1,0,0}.cross({0,1,0});
    EXPECT_NEAR(z.x, 0, 1e-6f);
    EXPECT_NEAR(z.y, 0, 1e-6f);
    EXPECT_NEAR(z.z, 1, 1e-6f);
    
    Vec3 mz = Vec3{0,1,0}.cross({1,0,0});
    EXPECT_NEAR(mz.z, -1, 1e-6f);
}

TEST(Vec3Test, CrossParallel) {
    Vec3 zero = Vec3{1,2,3}.cross({2,4,6});
    EXPECT_NEAR(zero.length(), 0, 1e-6f);
}

TEST(Vec3Test, Length) {
    EXPECT_NEAR(Vec3(3,4,0).length(),    5, 1e-6f);
    EXPECT_NEAR(Vec3(3,4,0).length_sq(), 25,1e-6f);
}

TEST(Vec3Test, Normalize) {
    Vec3 n = Vec3{3,4,0}.normalized();
    EXPECT_NEAR(n.length(), 1,    1e-6f);
    EXPECT_NEAR(n.x,        0.6f, 1e-6f);
    EXPECT_NEAR(n.y,        0.8f, 1e-6f);
}

TEST(Vec3Test, Negate) {
    Vec3 b = -Vec3{1,-2,3};
    EXPECT_NEAR(b.x, -1, 1e-6f);
    EXPECT_NEAR(b.y,  2, 1e-6f);
    EXPECT_NEAR(b.z, -3, 1e-6f);
}
