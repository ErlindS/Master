#include <gtest/gtest.h>
#include "obj_loader.h"
#include "scene.h"
#include <fstream>
#include <cstdio>

TEST(ObjLoaderTest, SimpleTriangle) {
    const char* path = "rt_test_tri.obj";
    { std::ofstream f(path);
      f << "v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nf 1//1 2//1 3//1\n"; }
    Scene sc;
    EXPECT_TRUE(load_obj(path, sc));
    EXPECT_EQ(sc.triangles.size(), 1);
    EXPECT_NEAR(sc.triangles[0].v1.x, 1.f, 1e-6f);
    std::remove(path); 
}

TEST(ObjLoaderTest, FanTriangulation) {
    const char* path = "rt_test_quad.obj";
    { std::ofstream f(path);
      f << "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n"; }
    Scene sc;
    load_obj(path, sc);
    EXPECT_EQ(sc.triangles.size(), 2);
    std::remove(path);
}

TEST(ObjLoaderTest, MissingFileFailsGracefully) {
    Scene sc;
    EXPECT_FALSE(load_obj("non_existent_file.obj", sc));
}
