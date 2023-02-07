#ifndef TINY_3DTILES_H_
#define TINY_3DTILES_H_
#include <string>
#include <vector>

#include "json.hpp"

namespace tiny3dtiles {
struct B3DM
{
    unsigned char magic[4];
    unsigned int version, byteLength, 
        featureTableJSONByteLength, featureTableBinaryByteLength, 
        batchTableJSONByteLength, batchTableBinaryByteLength;
    std::string featureTableJSON;
    std::string batchTableJSON;
    std::vector<unsigned char> glb;
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
    };
    Content content;
    std::vector<std::shared_ptr<Tile>> children;
    BoundingVolume boundingVolume;
    std::string refine;
    double geometricError;
    double transform[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0, 
        0, 0, 1, 0,
        0, 0, 0, 1};
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


