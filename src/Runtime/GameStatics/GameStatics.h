#pragma once

#include <string>
#include <vector>
#include <memory>

#include <tinygltf/tiny_gltf.h>

namespace nilou {

    class GameStatics
    {
    public:
        static bool StartsWith(const std::string &str, const std::string &temp);

        static bool EndsWith(const std::string &str, const std::string &temp);
        
        static void Trim(std::string &s);

        static std::vector<std::string> Split(const std::string &s, char delim = ' ');

        
        static std::vector<std::shared_ptr<class UStaticMesh>> ParseToStaticMeshes(tinygltf::Model &model);

        static std::vector<std::shared_ptr<class FMaterial>> ParseToMaterials(tinygltf::Model &model);
    };
}

