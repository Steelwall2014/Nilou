#ifndef TINY_3DTILES_H_
#define TINY_3DTILES_H_
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

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



#if defined(TINY3DTILES_IMPLEMENTATION) || defined(__INTELLISENSE__)

#include <fstream>
#include <filesystem>
#include <iostream>

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

void B3DM::load_header(const std::string &path)
{
    std::ifstream stream(path, std::ios::binary);
    load_header(stream);
}

void B3DM::load_header(std::istream &stream)
{
    binary_istream in(stream);
    in >> magic[0] >> magic[1] >> magic[2] >> magic[3];
    in >> version >> byteLength >> featureTableJSONByteLength >> featureTableBinaryByteLength >> batchTableJSONByteLength >> batchTableBinaryByteLength;
    gltfByteLength = 
            byteLength-28-
            featureTableJSONByteLength-batchTableJSONByteLength-
            featureTableBinaryByteLength-batchTableBinaryByteLength;
    featureTableJSON.resize(featureTableJSONByteLength);
    for (int i = 0; i < featureTableJSONByteLength; i++)
    {
        in >> featureTableJSON[i];
    }
    if (featureTableBinaryByteLength != 0)
    {
        featureTableBinary.resize(featureTableBinaryByteLength);
        for (int i = 0; i < featureTableBinaryByteLength; i++)
            in >> featureTableBinary[i];
        // featureTableBinary = std::make_shared<unsigned char>(featureTableBinaryByteLength);
        // stream.readsome((char*)featureTableBinary.get(), featureTableBinaryByteLength);
    }

    batchTableJSON.resize(batchTableJSONByteLength);
    for (int i = 0; i < batchTableJSONByteLength; i++)
    {
        in >> batchTableJSON[i];
    }
    if (batchTableBinaryByteLength != 0)
    {
        batchTableBinary.resize(batchTableBinaryByteLength);
        for (int i = 0; i < batchTableBinaryByteLength; i++)
            in >> batchTableBinary[i];
        // batchTableBinary = std::make_shared<unsigned char>(batchTableBinaryByteLength);
        // stream.readsome((char*)batchTableBinary.get(), batchTableBinaryByteLength);
    }
}

void B3DM::load_glb(const std::string &path)
{
    std::ifstream stream(path, std::ios::binary);
    load_glb(stream);
}

void B3DM::load_glb(std::istream &stream)
{
    if (!header_loaded())
    {
        load_header(stream);
    }
    else 
    {
        stream.seekg(byteLength-gltfByteLength);
    }
    binary_istream in(stream);

    if (gltfByteLength != 0)
    {
        glb.resize(gltfByteLength);
        for (int i = 0; i < gltfByteLength; i++)
            in >> glb[i];
        // glb = std::make_shared<unsigned char>(gltfByteLength);
        // stream.readsome((char*)glb.get(), gltfByteLength);
    }
}

void B3DM::load(const std::string &path) 
{ 
    std::ifstream stream(path, std::ios::binary);
    load(stream);
}
void B3DM::load(std::istream &stream) 
{ 
    load_header(stream); 
    load_glb(stream); 
}

void B3DM::reset_all()
{
    reset_header();
    reset_glb();
}

void B3DM::reset_header()
{
    std::memset(magic, 0, 4);
    version = byteLength = 
        featureTableJSONByteLength = featureTableBinaryByteLength = 
        batchTableJSONByteLength = batchTableBinaryByteLength = 0;
    featureTableJSON = "";
    featureTableBinary.clear();
    // if (featureTableBinary != nullptr)
    // {
    //     featureTableBinary = nullptr;
    // }
    batchTableJSON = "";
    batchTableBinary.clear();
    // if (batchTableBinary != nullptr)
    // {
    //     batchTableBinary = nullptr;
    // }
}

void B3DM::reset_glb()
{
    glb.clear();
    // if (glb != nullptr)
    // {
    //     glb = nullptr;
    // }
}

bool B3DM::header_loaded() const
{
    return magic[0] == 'b' && magic[1] == '3' && magic[2] == 'd' && magic[3] == 'm';
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

    if (tile_json.count("transform") != 0 && tile_json["transform"].is_array())
    {
        int i = 0;
        for (auto &element : tile_json["transform"])
        {
            if (element.is_number())
                tile->transform[i/4][i%4] = element.get<double>();
            i++;
        }
    }

    if (tile_json.count("refine") != 0 && tile_json["refine"].is_string())
    {
        std::string refine = tile_json["refine"].get<std::string>();
        if (refine == "REFINE")
            tile->refine = Tile::Refinement::REFINE;
        else if (refine == "ADD")
            tile->refine = Tile::Refinement::ADD;
    }

    bool content_is_tileset = false;
    if (tile_json.count("content") != 0 && tile_json["content"].is_object())
    {
        const json &content_json = tile_json["content"];
        if (content_json.count("boundingVolume") != 0)
            tile->content.boundingVolume.from_json(content_json["boundingVolume"]);
        if (content_json.count("uri") != 0)
        {            
            tile->content.uri = content_json["uri"].get<std::string>();
            fs::path path = fs::weakly_canonical(CurrentTilesetJSONFilePath.parent_path() / fs::path(tile->content.uri));
            tile->content.uri = path.generic_string();
            if (fs::exists(path))
            {            
                const std::string ext = GetFilePathExtension(tile->content.uri);
                if (ext == "b3dm")
                {
                    tile->content.b3dm.load_header(tile->content.uri);
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
    if (fs::exists(FilePath))
    {
        std::ifstream input_3dtiles_root_json_file(FilePath);
        nlohmann::json root_tileset_json;
        input_3dtiles_root_json_file >> root_tileset_json;
        return BuildTileset(root_tileset_json, fs::path(FilePath));
    }
    return nullptr;
}

}   // namespace tiny3dtiles


#endif  // TINY3DTILES_IMPLEMENTATION