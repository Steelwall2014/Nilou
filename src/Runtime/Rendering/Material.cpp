#include "Material.h"
#include "Common/AssetLoader.h"
#include "RHIDefinitions.h"
#include "Templates/ObjectMacros.h"
#include "Common/Path.h"
#include "Common/ContentManager.h"

namespace nilou {

    const std::filesystem::path MATERIAL_STATIC_PARENT_DIR = FPath::MaterialDir();

    FMaterial *FMaterial::GetDefaultMaterial()
    {
        return FContentManager::GetContentManager().GetGlobalMaterial("DefaultMaterial");
    }

    void FMaterial::UpdateMaterialCode(const std::string &InCode)
    {
        ParsedResult = FShaderParser(InCode, MATERIAL_STATIC_PARENT_DIR).Parse();
        FShaderCompiler::CompileMaterialShader(this);
    }
}