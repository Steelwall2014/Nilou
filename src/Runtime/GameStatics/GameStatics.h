#pragma once

#include <string>
#include <vector>
#include <memory>

#include <tinygltf/tiny_gltf.h>

namespace nilou {

    struct GLTFParseResult
    {
        std::vector<std::shared_ptr<class UStaticMesh>> StaticMeshes;
        std::vector<std::shared_ptr<class FMaterial>> Materials;
        std::vector<std::shared_ptr<class FTexture>> Textures;
    };

    class GameStatics
    {
    public:
        static bool StartsWith(const std::string &str, const std::string &temp);

        static bool EndsWith(const std::string &str, const std::string &temp);
        
        static void Trim(std::string &s);

        static std::vector<std::string> Split(const std::string &s, char delim = ' ');

        static GLTFParseResult ParseToStaticMeshes(tinygltf::Model &model);
    };
}

