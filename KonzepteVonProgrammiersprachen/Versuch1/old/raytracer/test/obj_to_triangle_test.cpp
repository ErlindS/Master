#include "gtest/gtest.h"
#include "obj_loader.h"
#include <sstream>
#include <stdexcept>
#include <fstream>
#include "obj_to_triangle.h"

using namespace wavefront;
using namespace geom;
using namespace math;

TEST(ObjToTriangle, QuadsWithNormalsAndComments) {
    std::istringstream is(
        "# This is a comment, anything after the '#' symbol is ignored\n"

        "# Define vertices\n"
        "v -0.5 -0.5 0.0\n"
        "v +0.5 -0.5 0.0\n"
        "v +0.5 +0.5 0.0\n"
        "v -0.5 +0.5 0.0\n"

        "# Define vertex normals\n"
        "vn 0.0 0.0 1.0\n"
        "vn 0.0 0.0 1.0\n"
        "vn 0.0 0.0 1.0\n"
        "vn 0.0 0.0 1.0\n"

        "# Define faces\n"
        "f 1//1 2//2 3//3\n"
        "f 3//3 4//4 1//1\n"

        "# Define another set of vertices\n"
        "v -1.0 -1.0 1.0\n"
        "v +1.0 -1.0 1.0\n"
        "v +1.0 +1.0 1.0\n"
        "v -1.0 +1.0 1.0\n"

        "# Define vertex normals for the new set of vertices\n"
        "vn 0.0 0.0 1.0\n"
        "vn 0.0 0.0 1.0\n"
        "vn 0.0 0.0 1.0\n"
        "vn 0.0 0.0 1.0\n"

        "# Define faces for the new set of vertices\n"
        "f 5//5 6//6 7//7\n"
        "f 7//7 8//8 5//5\n"
    );

    wavefront::ObjLoader loader(is);

    EXPECT_NO_THROW(loader.load(is)); // Make sure loading does not throw an exception
	
	 // Before calling setup_triangles, check if faces were loaded
    const auto& loadedFaces = loader.getFaces();
    
	ASSERT_NE(loadedFaces.size(), 0);
	
    std::vector<std::shared_ptr<TriangleWithNormals3f>> triangles = setup_triangles(loader);

    // check the resulting triangles
    ASSERT_EQ(triangles.size(), 4); // Expect 4 triangles (2 quads * 2 triangles/quad)

    // Check first triangle
    EXPECT_FLOAT_EQ(triangles[0]->vertices[0][0], -0.5f);
    EXPECT_FLOAT_EQ(triangles[0]->vertices[0][1], -0.5f);
    EXPECT_FLOAT_EQ(triangles[0]->vertices[0][2], 0.0f);

    EXPECT_FLOAT_EQ(triangles[0]->vertices[1][0], 0.5f);
    EXPECT_FLOAT_EQ(triangles[0]->vertices[1][1], -0.5f);
    EXPECT_FLOAT_EQ(triangles[0]->vertices[1][2], 0.0f);

    EXPECT_FLOAT_EQ(triangles[0]->vertices[2][0], 0.5f);
    EXPECT_FLOAT_EQ(triangles[0]->vertices[2][1], 0.5f);
    EXPECT_FLOAT_EQ(triangles[0]->vertices[2][2], 0.0f);

    EXPECT_FLOAT_EQ(triangles[0]->getNormal(0)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[0]->getNormal(0)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[0]->getNormal(0)[2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[0]->getNormal(1)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[0]->getNormal(1)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[0]->getNormal(1)[2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[0]->getNormal(2)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[0]->getNormal(2)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[0]->getNormal(2)[2], 1.0f);

    // Check second triangle
    EXPECT_FLOAT_EQ(triangles[1]->vertices[0][0], 0.5f);
    EXPECT_FLOAT_EQ(triangles[1]->vertices[0][1], 0.5f);
    EXPECT_FLOAT_EQ(triangles[1]->vertices[0][2], 0.0f);

    EXPECT_FLOAT_EQ(triangles[1]->vertices[1][0], -0.5f);
    EXPECT_FLOAT_EQ(triangles[1]->vertices[1][1], 0.5f);
    EXPECT_FLOAT_EQ(triangles[1]->vertices[1][2], 0.0f);

    EXPECT_FLOAT_EQ(triangles[1]->vertices[2][0], -0.5f);
    EXPECT_FLOAT_EQ(triangles[1]->vertices[2][1], -0.5f);
    EXPECT_FLOAT_EQ(triangles[1]->vertices[2][2], 0.0f);

    EXPECT_FLOAT_EQ(triangles[1]->getNormal(0)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[1]->getNormal(0)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[1]->getNormal(0)[2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[1]->getNormal(1)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[1]->getNormal(1)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[1]->getNormal(1)[2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[1]->getNormal(2)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[1]->getNormal(2)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[1]->getNormal(2)[2], 1.0f);

     // Check third triangle
    EXPECT_FLOAT_EQ(triangles[2]->vertices[0][0], -1.0f);
    EXPECT_FLOAT_EQ(triangles[2]->vertices[0][1], -1.0f);
    EXPECT_FLOAT_EQ(triangles[2]->vertices[0][2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[2]->vertices[1][0], 1.0f);
    EXPECT_FLOAT_EQ(triangles[2]->vertices[1][1], -1.0f);
    EXPECT_FLOAT_EQ(triangles[2]->vertices[1][2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[2]->vertices[2][0], 1.0f);
    EXPECT_FLOAT_EQ(triangles[2]->vertices[2][1], 1.0f);
    EXPECT_FLOAT_EQ(triangles[2]->vertices[2][2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[2]->getNormal(0)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[2]->getNormal(0)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[2]->getNormal(0)[2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[2]->getNormal(1)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[2]->getNormal(1)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[2]->getNormal(1)[2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[2]->getNormal(2)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[2]->getNormal(2)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[2]->getNormal(2)[2], 1.0f);

    // Check fourth triangle
    EXPECT_FLOAT_EQ(triangles[3]->vertices[0][0], 1.0f);
    EXPECT_FLOAT_EQ(triangles[3]->vertices[0][1], 1.0f);
    EXPECT_FLOAT_EQ(triangles[3]->vertices[0][2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[3]->vertices[1][0], -1.0f);
    EXPECT_FLOAT_EQ(triangles[3]->vertices[1][1], 1.0f);
    EXPECT_FLOAT_EQ(triangles[3]->vertices[1][2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[3]->vertices[2][0], -1.0f);
    EXPECT_FLOAT_EQ(triangles[3]->vertices[2][1], -1.0f);
    EXPECT_FLOAT_EQ(triangles[3]->vertices[2][2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[3]->getNormal(0)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[3]->getNormal(0)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[3]->getNormal(0)[2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[3]->getNormal(1)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[3]->getNormal(1)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[3]->getNormal(1)[2], 1.0f);

    EXPECT_FLOAT_EQ(triangles[3]->getNormal(2)[0], 0.0f);
    EXPECT_FLOAT_EQ(triangles[3]->getNormal(2)[1], 0.0f);
    EXPECT_FLOAT_EQ(triangles[3]->getNormal(2)[2], 1.0f);
}