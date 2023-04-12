#include "Common/Path.h"
#include "Material.h"
#include "Common/Asset/AssetLoader.h"
#include "RHIDefinitions.h"
#include "Templates/ObjectMacros.h"
#include "Common/Path.h"
#include "Common/ContentManager.h"
#include "RenderingThread.h"
#include <fstream>

namespace fs = std::filesystem;

namespace nilou {

    const std::filesystem::path MATERIAL_STATIC_PARENT_DIR = FPath::MaterialDir();

    void FMaterial::UpdateMaterialCode(const std::string &InCode, bool bRecompile)
    {
        if (bRecompile)
        {
            ENQUEUE_RENDER_COMMAND(UpdateMaterialCode)([this, InCode](FDynamicRHI *DynamicRHI) {
                std::string PreprocessResult = FShaderParser(InCode, MATERIAL_STATIC_PARENT_DIR).Parse();
                FShaderCompiler::CompileMaterialShader(this, PreprocessResult, DynamicRHI);
                bShaderCompiled = true;
            });
        }
    }

    UMaterial *UMaterial::GetDefaultMaterial()
    {
        return GetContentManager()->GetMaterialByPath("/Materials/DefaultMaterial.nasset");
    }

    void UMaterial::UpdateCode(const std::string &InCode, bool bRecompile)
    {
        Code = InCode;
        if (MaterialResource)
            MaterialResource->UpdateMaterialCode(Code, bRecompile);
    }

    void UMaterial::SetShaderFileVirtualPath(const std::filesystem::path& VirtualPath)
    {
        ShaderVirtualPath = VirtualPath;
        std::string ShaderAbsPath = GetShaderAbsolutePathFromVirtualPath(ShaderVirtualPath.generic_string());
        Code = GetAssetLoader()->SyncOpenAndReadText(ShaderAbsPath.c_str());
        UpdateCode(Code);
    }

    void UMaterial::Serialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        json["ClassName"] = "UMaterial";
        nlohmann::json &content = json["Content"];
        content["Name"] = Name;
        content["ShaderVirtualPath"] = ShaderVirtualPath.generic_string();
        content["ShadingModel"] = magic_enum::enum_name(ShadingModel);
        content["StencilRefValue"] = MaterialResource->StencilRefValue;
        TStaticSerializer<FBlendStateInitializer>::Serialize(MaterialResource->BlendState, content["BlendState"], Ar.OutBuffers);
        TStaticSerializer<FRasterizerStateInitializer>::Serialize(MaterialResource->RasterizerState, content["RasterizerState"], Ar.OutBuffers);
        TStaticSerializer<FDepthStencilStateInitializer>::Serialize(MaterialResource->DepthStencilState, content["DepthStencilState"], Ar.OutBuffers);
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
        if (content.contains("ShadingModel"))
        {
            SetShadingModel(magic_enum::enum_cast<EShadingModel>(content["ShadingModel"].get<std::string>()).value());
        }
        MaterialResource->Name = Name;
        MaterialResource->StencilRefValue = content["StencilRefValue"];
        TStaticSerializer<FBlendStateInitializer>::Deserialize(MaterialResource->BlendState, content["BlendState"], Ar.InBuffer.get());
        TStaticSerializer<FRasterizerStateInitializer>::Deserialize(MaterialResource->RasterizerState, content["RasterizerState"], Ar.InBuffer.get());
        TStaticSerializer<FDepthStencilStateInitializer>::Deserialize(MaterialResource->DepthStencilState, content["DepthStencilState"], Ar.InBuffer.get());
        
        std::string ShaderVirtualPath = content["ShaderVirtualPath"];
        SetShaderFileVirtualPath(ShaderVirtualPath);
        
        nlohmann::json &textures = content["Textures"];
        for (auto &[sampler_name, texture] : textures.items())
        {
            fs::path texture_path = texture.get<std::string>();
            UTexture *Texture = GetContentManager()->GetTextureByPath(texture_path);
            if (Texture)
                MaterialResource->SetParameterValue(sampler_name, Texture);
        }
    }

    UMaterialInstance* UMaterial::CreateMaterialInstance()
    {
        UMaterialInstance* MaterialInstance = new UMaterialInstance(this);
        return MaterialInstance;
    }

    UMaterialInstance::UMaterialInstance(UMaterial* Material)
    {
        Name = Material->Name;
        Code = Material->Code;
        SerializationPath = Material->SerializationPath;
        Textures = Material->Textures;
        SetShadingModel(Material->ShadingModel);
        MaterialResource->Name = Material->MaterialResource->Name;
        MaterialResource->StencilRefValue = Material->MaterialResource->StencilRefValue;
        MaterialResource->BlendState = Material->MaterialResource->BlendState;
        MaterialResource->DepthStencilState = Material->MaterialResource->DepthStencilState;
        MaterialResource->RasterizerState = Material->MaterialResource->RasterizerState;
        MaterialResource->ShaderMap = Material->MaterialResource->ShaderMap;
        MaterialResource->UniformBuffers = Material->MaterialResource->UniformBuffers;
        MaterialResource->Uniforms = Material->MaterialResource->Uniforms;
        MaterialResource->Textures = Material->MaterialResource->Textures;
        MaterialResource->bShaderCompiled = Material->MaterialResource->bShaderCompiled;
    }

    void UMaterialInstance::Serialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        UMaterial::Serialize(Ar);
        json["ClassName"] = "UMaterialInstance";
    }

}