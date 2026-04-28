#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "vector.h"
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <memory>

using namespace math;

namespace wavefront {
/**
 * @brief Represents a face in a Wavefront OBJ model, detailing vertex, texture, and normal indices.
 * 
 * @note Vertex, texture, and normal indices are stored as vectors.
 * @note Indices are converted from 1-based (as in OBJ files) to 0-based for internal use.
 * @note A value of -1 indicates that the corresponding data is not specified for that vertex.
 */
struct Face {
    std::vector<int> vertexIndices;      ///< Indices of the vertices in the vertices list (1-based in OBJ, converted to 0-based)
    std::vector<int> textureIndices;     ///< Indices of the texture coordinates, or -1 if not specified (range: -1 to max int)
    std::vector<int> normalIndices;      ///< Indices of the normals, or -1 if not specified (range: -1 to max int)
};

/**
 * @brief Enumeration for texture blending modes as defined by Wavefront's MTL format.
 */
enum class BlendMode {
    Modulate,   ///< Standard blending, where the texture modulates the material color
    Decal,      ///< Texture color is applied directly without modulation
    BlendU,     ///< Blend in U direction
    BlendV      ///< Blend in V direction
};

/**
 * @brief Represents a texture map with various options as defined in MTL files.
 * 
 * @note All attributes are optional and thus stored as unique_ptr to reflect this.
 */
struct TextureMap {
    std::string filename;           ///< Path to the texture file
    std::unique_ptr<bool> clamp;    ///< If true, texture coordinates are clamped instead of wrapped (range: {false, true})
    std::unique_ptr<float> bumpMultiplier; ///< Multiplier for bump mapping effect intensity (range: typically 0.0 to 10.0 or more)
    std::unique_ptr<BlendMode> blendMode;  ///< Specifies the blending mode for this texture
    std::unique_ptr<bool> blendU;   ///< Whether to blend in U direction; affects blendMode (range: {false, true})
    std::unique_ptr<bool> blendV;   ///< Whether to blend in V direction; affects blendMode (range: {false, true})
    std::unique_ptr<std::string> offset;    ///< Texture offset in U, V, W (if applicable) format (format: U V [W])
    std::unique_ptr<std::string> scale;     ///< Texture scaling factors in U, V, W (if applicable) format (format: U V [W])
    std::unique_ptr<std::string> turbulence;///< Parameters for texture turbulence effects

    TextureMap() = default;
};

/**
 * @brief Represents material properties as defined in Wavefront MTL files.
 * 
 * @note All attributes are optional and stored as unique_ptr to support materials with partial definitions.
 * @note Copy constructor and assignment operator are deleted to prevent copying of unique_ptr members.
 */
struct Material {
    std::string name;               ///< Name of the material
    std::unique_ptr<Vector3f> ambient;         ///< Ambient color (Ka) (each component range: 0.0 to 1.0)
    std::unique_ptr<Vector3f> diffuse;         ///< Diffuse color (Kd) (each component range: 0.0 to 1.0)
    std::unique_ptr<Vector3f> specular;        ///< Specular color (Ks) (each component range: 0.0 to 1.0)
    std::unique_ptr<float> shininess;          ///< Shininess or specular exponent (Ns) (range: 0.0 to 1000.0 or higher)
    std::unique_ptr<TextureMap> diffuseTexture;///< Diffuse texture map (map_Kd)
    std::unique_ptr<TextureMap> ambientTexture;///< Ambient texture map (map_Ka)
    std::unique_ptr<TextureMap> specularTexture;///< Specular texture map (map_Ks)
    std::unique_ptr<TextureMap> specularHighlightTexture; ///< Specular highlight texture map (map_Ns)
    std::unique_ptr<TextureMap> alphaTexture;  ///< Alpha texture map for transparency (map_d)
    std::unique_ptr<TextureMap> bumpMap;       ///< Bump map texture (map_Bump or bump)
    std::unique_ptr<float> transmission;       ///< Transparency or dissolve factor (d or Tr) (range: 0.0 for fully transparent to 1.0 for fully opaque)
    std::unique_ptr<float> opticalDensity;     ///< Optical density or index of refraction (Ni) (typically >= 1.0, where 1.0 is air)

    Material() = default;

    Material(Material&& other) noexcept = default; ///< Move constructor for efficient resource transfer
    Material& operator=(Material&& other) noexcept = default; ///< Move assignment operator

    Material(const Material&) = delete; ///< Deleted to prevent copying of materials with unique_ptr members
    Material& operator=(const Material&) = delete; ///< Deleted to prevent copying of materials with unique_ptr members
};

/**
 * @brief Represents a group of faces that share the same material.
 */
struct MaterialGroup {
    std::string materialName; ///< Name of the material this group is associated with
    std::vector<Face> faces;  ///< Faces that use this material

    MaterialGroup(const std::string& name) : materialName(name) {}
};


/**
 * @brief Class for loading and parsing Wavefront OBJ models, including materials from MTL files.
 * 
 * This class handles the parsing of OBJ files and associated MTL files for materials. 
 * It supports comments, material libraries, and various geometric and texture data.
 * 
 * @note Throws std::runtime_error when encountering parsing errors or file issues.
 */
class ObjLoader {
private:
    std::vector<Vector3f> vertices;      ///< Stores vertex positions from 'v' lines
    std::vector<Vector3f> normals;       ///< Stores vertex normals from 'vn' lines
    std::vector<Vector2f> texcoords;     ///< Stores texture coordinates from 'vt' lines
    std::vector<Face> faces;             ///< Stores face data from 'f' lines
    std::unordered_map<std::string, Material> materials;  ///< Stores materials from MTL files
    std::string currentMaterial;         ///< Keeps track of the current material name for 'usemtl'
    std::unordered_map<std::string, MaterialGroup> materialGroups; ///< Groups faces by material
    const std::string defaultMaterialName = "default_material_from_parser"; ///< Used if faces with no material are parsed.
    /**
     * @brief Splits a string into tokens based on a delimiter.
     * 
     * @param s The string to split.
     * @param delimiter The character used to split the string.
     * @return Vector of strings representing the split parts.
     */
    std::vector<std::string> split(const std::string &s, char delimiter);

    /**
     * @brief Parses a face string into a Face structure.  Handles n-gons.
     *
     * @param faceString String representation of a face from OBJ file (e.g., "1/1/1 2/2/2 3/3/3").
     * @return A Face struct with the parsed indices.
     * @throws std::runtime_error if the face string cannot be properly parsed or indices are out of range.
     */
    Face parseFace(const std::string& faceString);


     /**
     * @brief Parses texture options from the string part following the texture filename in MTL.
     * 
     * @param texture Reference to TextureMap to be populated with options.
     * @param options String containing texture options.
     * @throws std::runtime_error if options cannot be parsed.
     */
    void parseTextureOptions(TextureMap& texture, const std::string& options);

    /**
     * @brief Strips comments from a line in OBJ or MTL files.
     * 
     * @param line The line to process.
     * @return The line with comments removed.
     */
    std::string strip_comments(const std::string& line);

public:
    /**
     * @brief Constructor that immediately loads the OBJ data from the given input stream.
     * 
     * @param is Input stream containing OBJ data.
     */
    ObjLoader(std::istream& is) { load(is); }

    /**
     * @brief Forwarding constructor to support rvalue references.
     * 
     * @param is Rvalue reference to an input stream containing OBJ data.
     */
    ObjLoader(std::istream&& is) : ObjLoader(is) {}

    /**
     * @brief Loads a Wavefront OBJ model from an input stream, including parsing any referenced MTL files.
     * 
     * @param is Input stream with OBJ content.
     * @return True if loading was successful, false otherwise.
     * @throws std::runtime_error for various parsing errors or if an MTL file cannot be opened.
     */
    bool load(std::istream& is);

    /**
     * @brief Retrieves all vertices from the loaded OBJ model.
     * 
     * @return Const reference to vertices vector.
     */
    const std::vector<Vector3f>& getVertices() const { return vertices; }

    /**
     * @brief Retrieves all normals from the loaded OBJ model.
     * 
     * @return Const reference to normals vector.
     */
    const std::vector<Vector3f>& getNormals() const { return normals; }

    /**
     * @brief Retrieves all texture coordinates from the loaded OBJ model.
     * 
     * @return Const reference to texture coordinates vector.
     */
    const std::vector<Vector2f>& getTexCoords() const { return texcoords; }

    /**
     * @brief Retrieves all face data from the loaded OBJ model.
     * 
     * @return Const reference to faces vector.
     */
    const std::vector<Face>& getFaces() const { return faces; }

    /**
     * @brief Retrieves all materials from the loaded OBJ model.
     * 
     * @return Const reference to materials map.
     */
    const std::unordered_map<std::string, Material>& getMaterials() const { return materials; }

    /**
     * @brief Gets the name of the material currently being applied to faces.
     * 
     * @return Const reference to the current material name string.
     */
    const std::string& getCurrentMaterial() const { return currentMaterial; }

   /**
     * @brief Retrieves the groups of faces by material.
     * 
     * @return Const reference to the map of material names to MaterialGroups.
     */
    const std::unordered_map<std::string, MaterialGroup>& getMaterialGroups() const { return materialGroups; }

    /**
     * @brief Reads material definitions from an input stream typically from an MTL file.
     * 
     * @param is Input stream containing material definitions.
     * @return True if materials were successfully read, false otherwise.
     * @throws std::runtime_error if material data cannot be parsed.
     */
    bool readMaterial(std::istream& is);
	
	/**
     * @brief Creates a default material with default values if it does not already exist.
	 *
	 * @param material The new default material used by the loader.
     */
    void setDefaultMaterial(Material material);
	
	/**
     * @brief Gets the default material. If it is not yet created, it creates and return it.
     *
     * @return the default material used by the loader.
     */
    Material & getDefaultMaterial();
};

} // namespace wavefront

#endif // OBJ_LOADER_H