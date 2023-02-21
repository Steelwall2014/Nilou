#pragma once

#include <string>
#include <vector>
#include <memory>

#include <tinygltf/tiny_gltf.h>
#include "UniformBuffer.h"

namespace nilou {

    BEGIN_UNIFORM_BUFFER_STRUCT(FCesium3DTilesMaterialBlock)
        SHADER_PARAMETER(vec4, baseColorFactor)
        SHADER_PARAMETER(vec3, emissiveFactor)
        SHADER_PARAMETER(float, metallicFactor)
        SHADER_PARAMETER(float, roughnessFactor)
    END_UNIFORM_BUFFER_STRUCT()

    struct GLTFParseResult
    {
        std::vector<std::shared_ptr<class UStaticMesh>> StaticMeshes;
        std::vector<std::shared_ptr<class FMaterial>> Materials;
        std::vector<std::shared_ptr<class FTexture>> Textures;
        TUniformBufferRef<FCesium3DTilesMaterialBlock> UniformBuffer;
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

