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
        });
    }

    FMaterial::~FMaterial()
    {
        ENQUEUE_RENDER_COMMAND(FMaterial_Deconstructor)(
            [this](FDynamicRHI *DynamicRHI) 
            {
                ShaderMap.RemoveAllShaders();
            });
    }
}