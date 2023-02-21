#include "Material.h"
#include "Common/AssetLoader.h"
#include "RHIDefinitions.h"
#include "Templates/ObjectMacros.h"
#include "Common/Path.h"
#include "Common/ContentManager.h"
#include "RenderingThread.h"

namespace nilou {

    const std::filesystem::path MATERIAL_STATIC_PARENT_DIR = FPath::MaterialDir();

    FMaterial *FMaterial::GetDefaultMaterial()
    {
        return FContentManager::GetContentManager().GetGlobalMaterial("DefaultMaterial");
    }

    void FMaterial::UpdateMaterialCode(const std::string &InCode)
    {
        ParsedResult = FShaderParser(InCode, MATERIAL_STATIC_PARENT_DIR).Parse();
        ENQUEUE_RENDER_COMMAND(UpdateMaterialCode)([this](FDynamicRHI *DynamicRHI) {
            FShaderCompiler::CompileMaterialShader(this, DynamicRHI);
            bShaderCompiled = true;
        });
    }

    std::shared_ptr<FMaterialInstance> FMaterial::CreateMaterialInstance()
    {
        std::shared_ptr<FMaterialInstance> MaterialInstance = std::make_shared<FMaterialInstance>(MaterialName+"_Instance");
        MaterialInstance->BlendState = BlendState;
        MaterialInstance->DepthStencilState = DepthStencilState;
        MaterialInstance->RasterizerState = RasterizerState;
        MaterialInstance->ShaderMap = ShaderMap;
        MaterialInstance->UniformBuffers = UniformBuffers;
        MaterialInstance->Textures = Textures;
        MaterialInstance->bShaderCompiled = bShaderCompiled;
        MaterialInstance->bUseWorldOffset = bUseWorldOffset;
        return MaterialInstance;
    }

}