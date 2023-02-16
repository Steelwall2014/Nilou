#ifndef TINY_3DTILES_H_
#define TINY_3DTILES_H_
#include <string>
#include <vector>

#include "json.hpp"

#include "../glm/glm.hpp"

namespace tiny3dtiles {
struct B3DM
{
    char magic[4];
    unsigned int version, byteLength, 
        featureTableJSONByteLength, featureTableBinaryByteLength, 
        batchTableJSONByteLength, batchTableBinaryByteLength, gltfByteLength;
    std::vector<unsigned char> featureTableBinary;
    std::vector<unsigned char> batchTableBinary;
    std::vector<unsigned char> glb;
    std::string featureTableJSON;
    std::string batchTableJSON;
    void load_header(const std::string &path);
    void load_header(std::istream &stream);
    void load_glb(const std::string &path);
    void load_glb(std::istream &stream);
    void load(const std::string &path);
    void load(std::istream &stream);
    void reset_all();
    void reset_header();
    void reset_glb();
    bool header_loaded() const;
}; 

struct BoundingVolume
{
    std::vector<double> box;
    std::vector<double> region;
    std::vector<double> sphere;
    void from_json(nlohmann::json boundingVolume);
};

class Tile 
{
public:
    struct Content
    {
        BoundingVolume boundingVolume;
        std::string uri;
        std::shared_ptr<class Tileset> external_tileset = nullptr;
        B3DM b3dm;
    };
    enum class Refinement
    {
        REFINE,
        ADD
    };
    Content content;
    std::vector<std::shared_ptr<Tile>> children;
    BoundingVolume boundingVolume;
    Refinement refine = Refinement::REFINE;
    double geometricError;
    glm::dmat4 transform = glm::dmat4(1);
};

class Tileset 
{
public:
    std::map<std::string, std::string> asset;
    std::shared_ptr<Tile> root;
    double geometricError;
};

class Loader 
{
public:
    std::shared_ptr<Tileset> LoadTileset(const std::string &FilePath);

private:
    std::shared_ptr<Tile> BuildTile(const nlohmann::json &tile_json, const std::filesystem::path &CurrentTilesetJSONFilePath);
    std::shared_ptr<Tileset> BuildTileset(const nlohmann::json &tileset_json, const std::filesystem::path &CurrentTilesetJSONFilePath);
};

}

#endif  // TINY_3DTILES_H_



