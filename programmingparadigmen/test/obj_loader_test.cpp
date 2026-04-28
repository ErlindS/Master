#include "gtest/gtest.h"
#include "obj_loader.h"
#include <sstream>
#include <stdexcept>
#include <fstream>

using namespace wavefront;

// the tests currently are obsolete since the color Ka, Kd, Ks in the importer is averaging the color value
/*
TEST(ObjLoaderTest, MaterialDefinitions) {
    std::istringstream is(
        "newmtl Material1\n"
        "Ka 0.5 0.5 0.5\n"
        "Kd 0.8 0.8 0.8\n"
        "Ks 0.2 0.2 0.2\n"
        "Ns 10.0\n"
        "d 0.9\n"

        "newmtl Material2\n"
        "Ka 0.2 0.2 0.2\n"
        "Kd 0.5 0.5 0.5\n"
        "Ks 0.8 0.8 0.8\n"
        "Ns 5.0\n"
        "Tr 0.1\n"
    );

    // Create an alive istream for the constructor
    std::istringstream dummy_is("");
    ObjLoader loader(dummy_is); // Dummy OBJ loader to have an instance for reading materials

    EXPECT_NO_THROW(loader.readMaterial(is)); // Directly read the material definitions

    const auto& materials = loader.getMaterials();

    EXPECT_EQ(materials.size(), 3); // Expect 3 materials (2 + default)

    // Check Material1
    auto it = materials.find("Material1");
    ASSERT_NE(it, materials.end()); // Material exists

    EXPECT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.5f);

    EXPECT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 0.8f);

    EXPECT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.2f);

    EXPECT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 10.0f);

    EXPECT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 0.9f);

    // Check Material2
    it = materials.find("Material2");
    ASSERT_NE(it, materials.end()); // Material exists

    EXPECT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.2f);

    EXPECT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 0.5f);

    EXPECT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.8f);

    EXPECT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 5.0f);

    EXPECT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 0.1f); // 'Tr' is an alias for 'd' in MTL files
}

TEST(ObjLoaderTest, AdvancedMaterialDefinitions) {
    std::istringstream is(
        "# Define a new material\n"
        "newmtl Material1\n"

        "# Ambient, diffuse, and specular colors\n"
        "Ka 0.2 0.2 0.2\n"
        "Kd 0.8 0.8 0.8\n"
        "Ks 0.5 0.5 0.5\n"

        "# Specular exponent\n"
        "Ns 10.0\n"

        "# Dissolve (transparency) factor\n"
        "d 1.0\n"

        "# Illumination model (0 = Color on and Ambient off, 1 = Color on and Ambient on, etc.)\n"
        "illum 2\n"

        "# Texture map ( diffuse texture )\n"
        "map_Kd texture1.png\n"

        "# Bump map ( normal texture )\n"
        "map_bump bump1.png\n"
        "bump bump1.png\n"

        "# Displacement map ( height texture )\n"
        "map_d disp1.png\n"

        "# Reflection map ( spherical environment map )\n"
        "refl refl1.png\n"

        "# Define another material\n"
        "newmtl Material2\n"

        "# Ambient, diffuse, and specular colors\n"
        "Ka 0.5 0.5 0.5\n"
        "Kd 0.9 0.9 0.9\n"
        "Ks 0.8 0.8 0.8\n"

        "# Specular exponent\n"
        "Ns 5.0\n"

        "# Dissolve (transparency) factor\n"
        "d 0.9\n"

        "# Illumination model (0 = Color on and Ambient off, 1 = Color on and Ambient on, etc.)\n"
        "illum 1\n"

        "# Texture map ( diffuse texture )\n"
        "map_Kd texture2.png\n"

        "# Bump map ( normal texture )\n"
        "map_Bump bump2.png\n"
    );

    std::istringstream dummy_is("");
    ObjLoader loader(dummy_is); // Dummy OBJ loader to have an instance for reading materials

    EXPECT_NO_THROW(loader.readMaterial(is)); // Directly read the material definitions

    const auto& materials = loader.getMaterials();
    EXPECT_EQ(materials.size(), 3); // Expect 3 materials (2 + default)

    // Check Material1
    auto it = materials.find("Material1");
    ASSERT_NE(it, materials.end()); // Material exists

    EXPECT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.2f);

    EXPECT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 0.8f);

    EXPECT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.5f);

    EXPECT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 10.0f);

    EXPECT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 1.0f);

    EXPECT_NE(it->second.diffuseTexture, nullptr);
    EXPECT_EQ(it->second.diffuseTexture->filename, "texture1.png");

    EXPECT_NE(it->second.bumpMap, nullptr);
    EXPECT_EQ(it->second.bumpMap->filename, "bump1.png");

    EXPECT_NE(it->second.alphaTexture, nullptr);
    EXPECT_EQ(it->second.alphaTexture->filename, "disp1.png");

    // Note: 'refl' is not currently stored in the Material struct, so we'll skip it for now

    // Check Material2
    it = materials.find("Material2");
    ASSERT_NE(it, materials.end()); // Material exists

    EXPECT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.5f);

    EXPECT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 0.9f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 0.9f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 0.9f);

    EXPECT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.8f);

    EXPECT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 5.0f);

    EXPECT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 0.9f);

    EXPECT_NE(it->second.diffuseTexture, nullptr);
    EXPECT_EQ(it->second.diffuseTexture->filename, "texture2.png");

    EXPECT_NE(it->second.bumpMap, nullptr);
    EXPECT_EQ(it->second.bumpMap->filename, "bump2.png");
}

TEST(ObjLoaderTest, MaterialWithTextureOptions) {
    std::istringstream is(
        "# Example material definition for testing\n"

        "newmtl test_material\n"
        "Ka 0.2 0.2 0.2\n"
        "Kd 0.8 0.8 0.8\n"
        "Ks 0.5 0.5 0.5\n"
        "Ns 10.0\n"
        "d 1.0\n"
        "Ni 1.5\n"
        "map_Kd texture.jpg\n"
        "map_Ka ambient.jpg -clamp\n"
        "map_Ks specular.jpg -bm 0.5\n"
        "map_Ns shininess.jpg -blendu off\n"
        "map_d alpha.png\n"
        "map_Bump bump.jpg -bm 1.0\n"

        "newmtl partial_material\n"
        "Ka 1.0 0.0 0.0\n"
        "Ns 100.0\n"
    );

    std::istringstream dummy_is("");
    ObjLoader loader(dummy_is); // Dummy OBJ loader to have an instance for reading materials

    EXPECT_NO_THROW(loader.readMaterial(is)); // Directly read the material definitions

    const auto& materials = loader.getMaterials();
    EXPECT_EQ(materials.size(), 3); // Expect 3 materials (2 + default)

    // Check 'test_material'
    auto it = materials.find("test_material");
    ASSERT_NE(it, materials.end()); // Material exists

    EXPECT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.2f);

    EXPECT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 0.8f);

    EXPECT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.5f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.5f);

    EXPECT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 10.0f);

    EXPECT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 1.0f);

    EXPECT_NE(it->second.opticalDensity, nullptr);
    EXPECT_FLOAT_EQ(*it->second.opticalDensity, 1.5f);

    // Checking texture maps and options for 'test_material'
    EXPECT_NE(it->second.diffuseTexture, nullptr);
    EXPECT_EQ(it->second.diffuseTexture->filename, "texture.jpg");

    EXPECT_NE(it->second.ambientTexture, nullptr);
    EXPECT_EQ(it->second.ambientTexture->filename, "ambient.jpg");
    EXPECT_NE(it->second.ambientTexture->clamp, nullptr);
    EXPECT_TRUE(*it->second.ambientTexture->clamp);

    EXPECT_NE(it->second.specularTexture, nullptr);
    EXPECT_EQ(it->second.specularTexture->filename, "specular.jpg");
    EXPECT_NE(it->second.specularTexture->bumpMultiplier, nullptr);
    EXPECT_FLOAT_EQ(*it->second.specularTexture->bumpMultiplier, 0.5f);

    EXPECT_NE(it->second.specularHighlightTexture, nullptr);
    EXPECT_EQ(it->second.specularHighlightTexture->filename, "shininess.jpg");
    EXPECT_NE(it->second.specularHighlightTexture->blendU, nullptr);
    EXPECT_FALSE(*it->second.specularHighlightTexture->blendU);

    EXPECT_NE(it->second.alphaTexture, nullptr);
    EXPECT_EQ(it->second.alphaTexture->filename, "alpha.png");

    EXPECT_NE(it->second.bumpMap, nullptr);
    EXPECT_EQ(it->second.bumpMap->filename, "bump.jpg");
    EXPECT_NE(it->second.bumpMap->bumpMultiplier, nullptr);
    EXPECT_FLOAT_EQ(*it->second.bumpMap->bumpMultiplier, 1.0f);

    // Check 'partial_material'
    it = materials.find("partial_material");
    ASSERT_NE(it, materials.end()); // Material exists

    EXPECT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 1.0f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.0f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.0f);

    EXPECT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 100.0f);

    // Optional properties not set should still be nullptr for 'partial_material'
    EXPECT_EQ(it->second.diffuse, nullptr);
    EXPECT_EQ(it->second.specular, nullptr);
    EXPECT_EQ(it->second.transmission, nullptr);
    EXPECT_EQ(it->second.opticalDensity, nullptr);
    EXPECT_EQ(it->second.diffuseTexture, nullptr);
    EXPECT_EQ(it->second.ambientTexture, nullptr);
    EXPECT_EQ(it->second.specularTexture, nullptr);
    EXPECT_EQ(it->second.specularHighlightTexture, nullptr);
    EXPECT_EQ(it->second.alphaTexture, nullptr);
    EXPECT_EQ(it->second.bumpMap, nullptr);
}


TEST(ObjLoaderTest, LoadMaterialDefinitions) {
    std::istringstream is(
        "newmtl porcelain\n"
        "Ka 0.8 0.8 1.0\n"
        "Kd 1.0 1.0 1.0\n"
        "Ks 0.1 0.1 0.1\n"
        "Ns 5\n"
        "d 1.0\n"

        "newmtl clay\n"
        "Ka 0.4 0.2 0.1\n"
        "Kd 0.8 0.4 0.2\n"
        "Ks 0.05 0.025 0.0125\n"
        "Ns 2\n"
        "d 1.0\n"

        "newmtl glass\n"
        "Ka 0.0 0.0 0.0\n"
        "Kd 0.1 0.1 0.1\n"
        "Ks 0.9 0.9 0.9\n"
        "Ns 125\n"
        "d 0.2\n"
        "Ni 1.5\n"

        "newmtl brushed_aluminum\n"
        "Ka 0.3 0.3 0.3\n"
        "Kd 0.7 0.7 0.7\n"
        "Ks 0.2 0.2 0.2\n"
        "Ns 20\n"
        "d 1.0\n"
    );

    // Create an alive istream for the constructor
    std::istringstream dummy_is("");
    ObjLoader loader(dummy_is); // Dummy OBJ loader to have an instance for reading materials

    EXPECT_NO_THROW(loader.readMaterial(is)); // Directly read the material definitions

    const auto& materials = loader.getMaterials();

    EXPECT_EQ(materials.size(), 5); // Expect 5 materials including the default material

    // Check porcelain
    auto it = materials.find("porcelain");
    ASSERT_NE(it, materials.end()); // Material exists

    ASSERT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 1.0f);

    ASSERT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 1.0f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 1.0f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 1.0f);

    ASSERT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.1f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.1f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.1f);

    ASSERT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 5.0f);

    ASSERT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 1.0f);

    // Check clay
    it = materials.find("clay");
    ASSERT_NE(it, materials.end()); // Material exists

    ASSERT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.4f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.1f);

    ASSERT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 0.8f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 0.4f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 0.2f);

    ASSERT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.05f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.025f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.0125f);

    ASSERT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 2.0f);

    ASSERT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 1.0f);

    // Check glass
    it = materials.find("glass");
    ASSERT_NE(it, materials.end()); // Material exists

    ASSERT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.0f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.0f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.0f);

    ASSERT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 0.1f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 0.1f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 0.1f);

    ASSERT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.9f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.9f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.9f);

    ASSERT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 125.0f);

    ASSERT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 0.2f);

    ASSERT_NE(it->second.opticalDensity, nullptr);
    EXPECT_FLOAT_EQ(*it->second.opticalDensity, 1.5f);

    // Check brushed_aluminum
    it = materials.find("brushed_aluminum");
    ASSERT_NE(it, materials.end()); // Material exists

    ASSERT_NE(it->second.ambient, nullptr);
    EXPECT_FLOAT_EQ((*it->second.ambient)[0], 0.3f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[1], 0.3f);
    EXPECT_FLOAT_EQ((*it->second.ambient)[2], 0.3f);

    ASSERT_NE(it->second.diffuse, nullptr);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[0], 0.7f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[1], 0.7f);
    EXPECT_FLOAT_EQ((*it->second.diffuse)[2], 0.7f);

    ASSERT_NE(it->second.specular, nullptr);
    EXPECT_FLOAT_EQ((*it->second.specular)[0], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.specular)[1], 0.2f);
    EXPECT_FLOAT_EQ((*it->second.specular)[2], 0.2f);

    ASSERT_NE(it->second.shininess, nullptr);
    EXPECT_FLOAT_EQ(*it->second.shininess, 20.0f);

    ASSERT_NE(it->second.transmission, nullptr);
    EXPECT_FLOAT_EQ(*it->second.transmission, 1.0f);

    //Check that there are 4 materials
    EXPECT_EQ(materials.size(), 5);
}
*/