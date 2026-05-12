#include "gtest/gtest.h"
#include "shading_algorithms.h"



// Test Color class
TEST(ShadingAlgorithmsTest, ColorConstructorAndAccessors) {
    Color<float> color(0.5f, 0.6f, 0.7f);
    EXPECT_FLOAT_EQ(color.getRed(), 0.5f);
    EXPECT_FLOAT_EQ(color.getGreen(), 0.6f);
    EXPECT_FLOAT_EQ(color.getBlue(), 0.7f);
}

// Test Material Constructor
TEST(ShadingAlgorithmsTest, MaterialConstructor) {
	auto material = Material3f(Color<float>{1, 1, 1}, 0.1f, 0.5f, 0.4f, 10.0f, 0.3f, 0.2f, 1.5f);
    EXPECT_FLOAT_EQ(material.color.getRed(), 1.0f);
    EXPECT_FLOAT_EQ(material.ka, 0.1f);
    EXPECT_FLOAT_EQ(material.kd, 0.5f);
    EXPECT_FLOAT_EQ(material.ks, 0.4f);
    EXPECT_FLOAT_EQ(material.shininess, 10.0f);
    EXPECT_FLOAT_EQ(material.reflectivity, 0.3f);
    EXPECT_FLOAT_EQ(material.transmission, 0.2f);
    EXPECT_FLOAT_EQ(material.opticalDensity, 1.5f);
}

// Test Light Constructor
TEST(ShadingAlgorithmsTest, LightConstructor) {
	auto light = Light3f(Vector<float, 3>{-1, 1, 1}, Vector<float, 3>{1, 1, 1}, Color<float>{1, 1, 1});
    EXPECT_EQ(light.position, (Vector<float, 3>{-1, 1, 1}));
    EXPECT_EQ(light.intensity, (Vector<float, 3>{1, 1, 1}));
    EXPECT_EQ(light.color, (Color<float>{1, 1, 1}));
}

// Test Lambertian Shading with one light
TEST(ShadingAlgorithmsTest, LambertianShadingOneLight) {
    auto material = Material3f(Color<float>{1, 1, 1}, 0.1f, 0.5f, 0.4f, 10.0f, 0.3f, 0.2f, 1.5f);
    auto light = Light3f(Vector<float, 3>{0, 0, 1}, Vector<float, 3>{1, 1, 1}, Color<float>{1, 1, 1});
    auto point = Vector<float, 3>{0, 0, 0};
	auto normal = Vector<float, 3>{0, 0, 1};   
	Intersection<float, 3> intersection;
    intersection.normal = normal;
    Ray<float, 3> ray{Vector<float, 3>{0, 0, -1}, Vector<float, 3>{0, 0, 1} }; // Direction towards the intersection
    ShadingAlgorithms<float, 3> shading;
    Color<float> result = shading.lambertianShading(point, intersection, ray, {light}, material);

    // Here we are checking if the color is at least influenced by the material's diffuse  components
    EXPECT_FLOAT_EQ(result.getRed(), 0.5f); // 0.5 * 1 (diffuse with full light incidence)
    EXPECT_FLOAT_EQ(result.getGreen(), 0.5f);
    EXPECT_FLOAT_EQ(result.getBlue(), 0.5f);
}


// Test Lambertian Shading with multiple lights
TEST(ShadingAlgorithmsTest, LambertianShadingMultipleLights) {
    auto material = Material3f(Color<float>{1, 1, 1}, 0.1f, 0.5f, 0.4f, 10.0f, 0.3f, 0.2f, 1.5f);
    auto light = Light3f(Vector<float, 3>{0, 0, 1}, Vector<float, 3>{1, 1, 1}, Color<float>{1, 1, 1});
    auto normal = Vector<float, 3>{0, 0, 1};  
    std::vector<Light3f> lights = {light, Light3f(Vector<float, 3>{0, 0, 1}, Vector<float, 3>{1, 1, 1}, Color<float>{1, 1, 1})};
    Intersection<float, 3> intersection;
    intersection.normal = normal;
    Ray<float, 3> ray(Vector<float, 3>{0, 0, -1}, Vector<float, 3>{0, 0, 1}); // Direction towards the intersection
    ShadingAlgorithms<float, 3> shading;
    Color<float> result = shading.lambertianShading(intersection, ray, lights, material);
    
    // Since both lights are above the surface, we expect twice the light contribution
    EXPECT_FLOAT_EQ(result.getRed(), 1.1f); // 0.1 (ambient) + 2 * 0.5 * 1 (diffuse with full light incidence from two sources)
    EXPECT_FLOAT_EQ(result.getGreen(), 1.1f);
    EXPECT_FLOAT_EQ(result.getBlue(), 1.1f);
}

// Test Phong Shading
TEST(ShadingAlgorithmsTest, PhongShading) {
    auto material = Material3f(Color<float>{1, 1, 1}, 0.1f, 0.5f, 0.4f, 10.0f, 0.3f, 0.2f, 1.5f);
    auto light = Light3f(Vector<float, 3>{0, 0, 1}, Vector<float, 3>{1, 1, 1}, Color<float>{1, 1, 1});
    auto normal = Vector<float, 3>{0, 0, 1};  
    auto point = Vector<float, 3>{0, 0, 0};
	Intersection<float, 3> intersection;
    intersection.normal = normal;
    Ray<float, 3> ray{Vector<float, 3>{0, 0, -1}, Vector<float, 3>{0, 0, 1}}; // Direction towards the intersection
    Vector<float, 3> viewDir = (Vector<float, 3>{0, 0, 1} - point).normalized();
    std::vector<Light3f> lights = {light};
    ShadingAlgorithms<float, 3> shading;
    Color<float> result = shading.phongShading(intersection, ray, lights, viewDir, material);
    
    // This is a simplified test; exact values depend on implementation details like light position and normal vector
    EXPECT_GT(result.getRed(), 0.1f); // Should be greater than just ambient due to diffuse and specular contributions
    EXPECT_GT(result.getGreen(), 0.1f);
    EXPECT_GT(result.getBlue(), 0.1f);
}
