#include <fstream>
#include <filesystem>
#include <iostream>

#include "tiny_3dtiles.h"

#define TINY3DTILES_IMPLEMENTATION

#if defined(TINY3DTILES_IMPLEMENTATION) || defined(__INTELLISENSE__)

using json = nlohmann::json;

namespace fs = std::filesystem;

namespace tiny3dtiles {


class binary_istream
{
public:
    binary_istream(std::istream &stream)
        : in(&stream)
    {

    }
    template<class T>
    binary_istream &operator>>(T &value)
    {
        in->read(reinterpret_cast<char*>(&value), sizeof(T));
        return *this;
    }
private:
    std::istream *in;
};

class binary_ostream
{
public:
    binary_ostream(std::ostream &stream)
        : out(&stream)
    {

    }
    template<class T>
    binary_ostream &operator<<(T value)
    {
        out->write(reinterpret_cast<char*>(&value), sizeof(T));
        return *this;
    }
private:
    std::ostream *out;
};

std::string GetFilePathExtension(const std::string &FileName)
{
  if (FileName.find_last_of(".") != std::string::npos)
    return FileName.substr(FileName.find_last_of(".") + 1);
  return "";
}

std::istream &operator>>(std::istream &stream, B3DM &b3dm)
{
    binary_istream in(stream);
    in >> b3dm.magic[0] >> b3dm.magic[1] >> b3dm.magic[2] >> b3dm.magic[3];
    in >> b3dm.version >> b3dm.byteLength >> b3dm.featureTableJSONByteLength >> b3dm.featureTableBinaryByteLength >> b3dm.batchTableJSONByteLength >> b3dm.batchTableBinaryByteLength;
    b3dm.featureTableJSON.resize(b3dm.featureTableJSONByteLength);
    for (int i = 0; i < b3dm.featureTableJSONByteLength; i++)
    {
        in >> b3dm.featureTableJSON[i];
    }
    b3dm.batchTableJSON.resize(b3dm.batchTableJSONByteLength);
    for (int i = 0; i < b3dm.batchTableJSONByteLength; i++)
    {
        in >> b3dm.batchTableJSON[i];
    }
    int gltf_size = b3dm.byteLength-b3dm.batchTableJSONByteLength-b3dm.featureTableJSONByteLength-28;
    b3dm.glb.resize(gltf_size);
    for (int i = 0; i < gltf_size; i++)
    {
        in >> b3dm.glb[i];
    }
    return stream;
}

void BoundingVolume::from_json(json boundingVolume)
{
    if (boundingVolume.is_object())
    {
        if (boundingVolume.count("box") == 1 && boundingVolume["box"].is_array())
        {
            this->box.reserve(boundingVolume["box"].size());
            for (auto &element : boundingVolume["box"])
            {
                if (element.is_number())
                    this->box.push_back(element.get<double>());
            }
        }
        if (boundingVolume.count("region") == 1 && boundingVolume["region"].is_array())
        {
            this->box.reserve(boundingVolume["region"].size());
            for (auto &element : boundingVolume["region"])
            {
                if (element.is_number())
                    this->region.push_back(element.get<double>());
            }
        }
        if (boundingVolume.count("sphere") == 1 && boundingVolume["sphere"].is_array())
        {
            this->box.reserve(boundingVolume["sphere"].size());
            for (auto &element : boundingVolume["sphere"])
            {
                if (element.is_number())
                    this->sphere.push_back(element.get<double>());
            }
        }
    }
}

std::shared_ptr<Tile> Loader::BuildTile(const nlohmann::json &tile_json, const fs::path &CurrentTilesetJSONFilePath)
{
    std::shared_ptr<Tile> tile = std::make_shared<Tile>();

    if (tile_json.count("geometricError") != 0 && tile_json["geometricError"].is_number())
        tile->geometricError = tile_json["geometricError"].get<double>();
    else 
        std::cout << "[ERROR] tile MUST have geometricError property\n";

    if (tile_json.count("boundingVolume") != 0 && tile_json["boundingVolume"].is_object())
        tile->boundingVolume.from_json(tile_json["boundingVolume"]);
    else 
        std::cout << "[ERROR] tile MUST have boundingVolume property\n";

    if (tile_json.count("refine") != 0 && tile_json["refine"].is_string())
        tile->refine = tile_json["refine"].get<std::string>();

    bool content_is_tileset = false;
    if (tile_json.count("content") != 0 && tile_json["content"].is_object())
    {
        const json &content_json = tile_json["content"];
        if (content_json.count("boundingVolume") != 0)
            tile->content.boundingVolume.from_json(content_json["boundingVolume"]);
        if (content_json.count("uri") != 0)
        {            
            tile->content.uri = content_json["uri"].get<std::string>();
            const std::string &uri = tile->content.uri;
            fs::path path = fs::canonical(CurrentTilesetJSONFilePath.parent_path() / fs::path(uri));
            const std::string ext = GetFilePathExtension(uri);
            if (ext == "b3dm")
            {
                std::ifstream blob(path);
                tile->content.b3dm = std::make_shared<B3DM>();
                blob >> *tile->content.b3dm;
            }
            else if (ext == "json")
            {
                content_is_tileset = true;
                std::ifstream tileset_json_file(path);
                nlohmann::json tileset_json;
                tileset_json_file >> tileset_json;
                tile->content.external_tileset = BuildTileset(tileset_json, path);
            }
        }
        else 
        {
            std::cout << "[ERROR] tile.content MUST have uri property\n";
        }

    }

    if (!content_is_tileset && tile_json.count("children") != 0 && tile_json["children"].is_array())
    {
        for (auto &child : tile_json["children"])
        {
            tile->children.push_back(BuildTile(child, CurrentTilesetJSONFilePath));
        }
    }

    return tile;
}

std::shared_ptr<Tileset> Loader::BuildTileset(const nlohmann::json &tileset_json, const fs::path &CurrentTilesetJSONFilePath)
{
    std::shared_ptr<Tileset> tileset = std::make_shared<Tileset>();
    if (tileset_json.count("asset") != 0 && tileset_json["asset"].is_object())
    {
        for (auto [key, value] : tileset_json["asset"].items())
        {
            tileset->asset[key] = value;
        }
    }
    if (tileset_json.count("geometricError") != 0 && tileset_json["geometricError"].is_number())
    {
        tileset->geometricError = tileset_json["geometricError"].get<double>();
    }
    if (tileset_json["root"] != 0 && tileset_json["root"].is_object())
    {
        tileset->root = BuildTile(tileset_json["root"], CurrentTilesetJSONFilePath);
    }
    return tileset;
}

std::shared_ptr<Tileset> Loader::LoadTileset(const std::string &FilePath)
{
    std::ifstream input_3dtiles_root_json_file(FilePath);
    nlohmann::json root_tileset_json;
    input_3dtiles_root_json_file >> root_tileset_json;
    return BuildTileset(root_tileset_json, fs::path(FilePath));
}

}   // namespace tiny3dtiles


#endif  // TINY3DTILES_IMPLEMENTATION