#include "Material.h"
#include "Common/AssetLoader.h"
#include "RHIDefinitions.h"
#include "Templates/ObjectMacros.h"
#include "Common/Path.h"
#include "Common/ContentManager.h"
#include "RenderingThread.h"

namespace nilou {

    const std::filesystem::path MATERIAL_STATIC_PARENT_DIR = FPath::MaterialDir();

    void FMaterial::UpdateMaterialCode(const std::string &InCode, bool bRecompile)
    {
        FShaderParserResult ParsedResult = FShaderParser(InCode, MATERIAL_STATIC_PARENT_DIR).Parse();
        if (bRecompile)
        {
            ENQUEUE_RENDER_COMMAND(UpdateMaterialCode)([this, &ParsedResult](FDynamicRHI *DynamicRHI) {
                FShaderCompiler::CompileMaterialShader(this, ParsedResult, DynamicRHI);
                bShaderCompiled = true;
            });
        }
    }

    UMaterial *UMaterial::GetDefaultMaterial()
    {
        return FContentManager::GetContentManager().GetGlobalMaterial("DefaultMaterial");
    }

    void UMaterial::UpdateCode(const std::string &InCode, bool bRecompile)
    {
        Code = InCode;
        if (MaterialResource)
            MaterialResource->UpdateMaterialCode(Code, bRecompile);
    }

    void UMaterial::Serialize(nlohmann::json &json)
    {
        json["ClassName"] = "UMaterial";
        nlohmann::json &content = json["Content"];
        content["Name"] = Name;
        content["Code"] = Code;
        content["StencilRefValue"] = MaterialResource->StencilRefValue;
        TStaticSerializer<FBlendStateInitializer>::Serialize(MaterialResource->BlendState, content["BlendState"]);
        TStaticSerializer<FRasterizerStateInitializer>::Serialize(MaterialResource->RasterizerState, content["RasterizerState"]);
        TStaticSerializer<FDepthStencilStateInitializer>::Serialize(MaterialResource->DepthStencilState, content["DepthStencilState"]);
    }

    void UMaterial::Deserialize(nlohmann::json &json)
    {
        if (!SerializeHelper::CheckIsType(json, "UMaterial") && 
            !SerializeHelper::CheckIsType(json, "UMaterialInstance")) return;
        nlohmann::json &content = json["Content"];
        Name = content["Name"];
        Code = content["Code"];
        MaterialResource->StencilRefValue = content["StencilRefValue"];
        TStaticSerializer<FBlendStateInitializer>::Deserialize(MaterialResource->BlendState, content["BlendState"]);
        TStaticSerializer<FRasterizerStateInitializer>::Deserialize(MaterialResource->RasterizerState, content["RasterizerState"]);
        TStaticSerializer<FDepthStencilStateInitializer>::Deserialize(MaterialResource->DepthStencilState, content["DepthStencilState"]);
        MaterialResource->UpdateMaterialCode(Code);
    }

    std::shared_ptr<UMaterialInstance> UMaterial::CreateMaterialInstance()
    {
        std::shared_ptr<UMaterialInstance> MaterialInstance = std::make_shared<UMaterialInstance>();
        MaterialInstance->Name = Name;
        MaterialInstance->Code = Code;
        MaterialInstance->Path = Path;
        MaterialInstance->UniformBuffers = UniformBuffers;
        MaterialInstance->Textures = Textures;
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

    void UMaterialInstance::Serialize(nlohmann::json &json)
    {
        UMaterial::Serialize(json);
        json["ClassName"] = "UMaterialInstance";
    }

}