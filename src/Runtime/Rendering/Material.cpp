#include "Common/Path.h"
#include "Material.h"
#include "Common/AssetLoader.h"
#include "RHIDefinitions.h"
#include "Templates/ObjectMacros.h"
#include "Common/Path.h"
#include "Common/ContentManager.h"
#include "RenderingThread.h"

namespace fs = std::filesystem;

namespace nilou {

    const std::filesystem::path MATERIAL_STATIC_PARENT_DIR = FPath::MaterialDir();

    void FMaterial::UpdateMaterialCode(const std::string &InCode, bool bRecompile)
    {
        if (bRecompile)
        {
            ENQUEUE_RENDER_COMMAND(UpdateMaterialCode)([this, InCode](FDynamicRHI *DynamicRHI) {
                FShaderParserResult ParsedResult = FShaderParser(InCode, MATERIAL_STATIC_PARENT_DIR).Parse();
                FShaderCompiler::CompileMaterialShader(this, ParsedResult, DynamicRHI);
                bShaderCompiled = true;
            });
        }
    }

    UMaterial *UMaterial::GetDefaultMaterial()
    {
        return GetContentManager()->GetMaterialByPath("/Materials/DefaultMaterial.json");
    }

    void UMaterial::UpdateCode(const std::string &InCode, bool bRecompile)
    {
        Code = InCode;
        if (MaterialResource)
            MaterialResource->UpdateMaterialCode(Code, bRecompile);
    }

    void UMaterial::Serialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        json["ClassName"] = "UMaterial";
        nlohmann::json &content = json["Content"];
        content["Name"] = Name;
        content["Code"] = Code;
        content["StencilRefValue"] = MaterialResource->StencilRefValue;
        TStaticSerializer<FBlendStateInitializer>::Serialize(MaterialResource->BlendState, content["BlendState"]);
        TStaticSerializer<FRasterizerStateInitializer>::Serialize(MaterialResource->RasterizerState, content["RasterizerState"]);
        TStaticSerializer<FDepthStencilStateInitializer>::Serialize(MaterialResource->DepthStencilState, content["DepthStencilState"]);
        nlohmann::json &textures = content["Textures"];
        for (auto &[Name, Texture] : Textures)
        {
            if (!Texture.empty())
            {
                textures[Name] = Texture.generic_string();
            }
        }
    }

    void UMaterial::Deserialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        if (!SerializeHelper::CheckIsType(json, "UMaterial") && 
            !SerializeHelper::CheckIsType(json, "UMaterialInstance")) return;
        nlohmann::json &content = json["Content"];
        Name = content["Name"];
        Code = content["Code"];
        MaterialResource->Name = Name;
        MaterialResource->StencilRefValue = content["StencilRefValue"];
        TStaticSerializer<FBlendStateInitializer>::Deserialize(MaterialResource->BlendState, content["BlendState"]);
        TStaticSerializer<FRasterizerStateInitializer>::Deserialize(MaterialResource->RasterizerState, content["RasterizerState"]);
        TStaticSerializer<FDepthStencilStateInitializer>::Deserialize(MaterialResource->DepthStencilState, content["DepthStencilState"]);
        MaterialResource->UpdateMaterialCode(Code);
        nlohmann::json &textures = content["Textures"];
        for (auto &[sampler_name, texture] : textures.items())
        {
            fs::path texture_path = texture.get<std::string>();
            UTexture *Texture = GetContentManager()->GetTextureByPath(texture_path);
            MaterialResource->SetParameterValue(sampler_name, Texture);
        }
    }

    std::shared_ptr<UMaterialInstance> UMaterial::CreateMaterialInstance()
    {
        std::shared_ptr<UMaterialInstance> MaterialInstance = std::make_shared<UMaterialInstance>();
        MaterialInstance->Name = Name;
        MaterialInstance->Code = Code;
        MaterialInstance->SerializationPath = SerializationPath;
        MaterialInstance->Textures = Textures;
        MaterialInstance->MaterialResource->Name = MaterialResource->Name;
        MaterialInstance->MaterialResource->StencilRefValue = MaterialResource->StencilRefValue;
        MaterialInstance->MaterialResource->BlendState = MaterialResource->BlendState;
        MaterialInstance->MaterialResource->DepthStencilState = MaterialResource->DepthStencilState;
        MaterialInstance->MaterialResource->RasterizerState = MaterialResource->RasterizerState;
        MaterialInstance->MaterialResource->ShaderMap = MaterialResource->ShaderMap;
        MaterialInstance->MaterialResource->UniformBuffers = MaterialResource->UniformBuffers;
        MaterialInstance->MaterialResource->Textures = MaterialResource->Textures;
        MaterialInstance->MaterialResource->bShaderCompiled = MaterialResource->bShaderCompiled;
        // MaterialInstance->bUseWorldOffset = bUseWorldOffset;
        return MaterialInstance;
    }

    void UMaterialInstance::Serialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        UMaterial::Serialize(Ar);
        json["ClassName"] = "UMaterialInstance";
    }

}