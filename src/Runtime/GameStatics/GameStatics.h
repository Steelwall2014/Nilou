#pragma once

#include <string>
#include <vector>
#include <memory>

#include <tinygltf/tiny_gltf.h>
#include "UniformBuffer.h"

namespace nilou {

    BEGIN_UNIFORM_BUFFER_STRUCT(FGLTFMaterialBlock)
        SHADER_PARAMETER(vec4, baseColorFactor)
        SHADER_PARAMETER(vec3, emissiveFactor)
        SHADER_PARAMETER(float, metallicFactor)
        SHADER_PARAMETER(float, roughnessFactor)
    END_UNIFORM_BUFFER_STRUCT()
    
    // template<>
    // class TStaticSerializer<FGLTFMaterialBlock>
    // {
    // public:
    //     static void Serialize(const FGLTFMaterialBlock &Object, nlohmann::json &json)
    //     {
    //         json["ClassName"] = "FGLTFMaterialBlock";
    //         nlohmann::json &content = json["Content"];
    //         content["baseColorFactor"] = {Object.baseColorFactor.r, Object.baseColorFactor.g, Object.baseColorFactor.b, Object.baseColorFactor.a};
    //         content["emissiveFactor"] = {Object.emissiveFactor.r, Object.emissiveFactor.g, Object.emissiveFactor.b};
    //         content["metallicFactor"] = Object.metallicFactor;
    //         content["roughnessFactor"] = Object.roughnessFactor;
    //     }
    //     static void Deserialize(FGLTFMaterialBlock &Object, nlohmann::json &json)
    //     {
    //         if (!SerializeHelper::CheckIsType(json, "FGLTFMaterialBlock")) return;
            
    //         nlohmann::json &content = json["Content"];
    //         Object.baseColorFactor.r = content["baseColorFactor"][0];
    //         Object.baseColorFactor.g = content["baseColorFactor"][1];
    //         Object.baseColorFactor.b = content["baseColorFactor"][2];
    //         Object.baseColorFactor.a = content["baseColorFactor"][3];
    //         Object.emissiveFactor.r = content["emissiveFactor"][0];
    //         Object.emissiveFactor.g = content["emissiveFactor"][1];
    //         Object.emissiveFactor.b = content["emissiveFactor"][2];
    //         Object.metallicFactor = content["metallicFactor"];
    //         Object.roughnessFactor = content["roughnessFactor"];
            
    //     }
    // };

    struct GLTFParseResult
    {
        std::vector<std::shared_ptr<class UStaticMesh>> StaticMeshes;
        std::vector<std::shared_ptr<class UMaterialInstance>> Materials;
        std::vector<std::shared_ptr<class UTexture>> Textures;
        TUniformBufferRef<FGLTFMaterialBlock> UniformBuffer;
        void InitResource();
    };

    class GameStatics
    {
    public:
        static bool StartsWith(const std::string &str, const std::string &temp);

        static bool EndsWith(const std::string &str, const std::string &temp);
        
        static void Trim(std::string &s);

        static std::vector<std::string> Split(const std::string &s, char delim = ' ');

        static GLTFParseResult ParseToStaticMeshes(tinygltf::Model &model, bool need_init=true);
    };
}

