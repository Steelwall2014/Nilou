#ifndef TINY_3DTILES_H_
#define TINY_3DTILES_H_
#include <string>
#include <vector>

#include "json.hpp"

#include "../glm/glm.hpp"

namespace tiny3dtiles {
struct B3DM
{
    unsigned char magic[4];
    unsigned int version, byteLength, 
        featureTableJSONByteLength, featureTableBinaryByteLength, 
        batchTableJSONByteLength, batchTableBinaryByteLength;
    std::string featureTableJSON;
    std::vector<unsigned char> featureTableBinary;
    std::string batchTableJSON;
    std::vector<unsigned char> batchTableBinary;
    std::vector<unsigned char> glb;
    void reset();
    friend std::istream &operator>>(std::istream &stream, B3DM &b3dm);
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
        std::shared_ptr<B3DM> b3dm = nullptr;
        void load_b3dm();
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



