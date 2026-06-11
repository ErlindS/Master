#include <gtest/gtest.h>
#include "mesh.h"
#include "scene.h"
#include <fstream>
#include <cstdio>

TEST(ObjLoaderTest, SimpleTriangle) {
    const char* path = "rt_test_tri.obj";
    { std::ofstream f(path);
      f << "v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nf 1//1 2//1 3//1\n"; }
    Mesh m;
    EXPECT_TRUE(m.loadOBJ(path));
    EXPECT_EQ(m.triangles.size(), 1);
    EXPECT_NEAR(m.triangles[0].v1.x, 1.f, 1e-6f);
    std::remove(path); 
}

TEST(ObjLoaderTest, FanTriangulation) {
    const char* path = "rt_test_quad.obj";
    { std::ofstream f(path);
      f << "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n"; }
    Mesh m;
    m.loadOBJ(path);
    EXPECT_EQ(m.triangles.size(), 2);
    std::remove(path);
}

TEST(ObjLoaderTest, MissingFileFailsGracefully) {
    Mesh m;
    EXPECT_FALSE(m.loadOBJ("non_existent_file.obj"));
}
