#include <regex>
#include <sstream>
#include <string>
#include "DynamicRHI.h"
#include "GameStatics.h"
#include "ShaderType.h"
#include "ShaderCompiler.h"
#include "Common/Log.h"
#include "Common/Asset/AssetLoader.h"
// #include "Shadinclude.h"
#include "ShaderPreprocess.h"

namespace nilou {


    std::vector<FShaderType *> &GetAllShaderTypes()
    {
    	static std::vector<FShaderType *> GAllShaderTypes;
        return GAllShaderTypes;
    }

    std::vector<FVertexFactoryType *> &GetAllVertexFactoryTypes()
    {
        static std::vector<FVertexFactoryType *> GAllVertexFactoryTypes;
        return GAllVertexFactoryTypes;
    }

    FShaderTypeBase::FShaderTypeBase(const std::string &InClassName, const std::string &InVirtualFilePath, int32 InPermutationCount)
        : Name(InClassName)
        , VirtualFilePath(InVirtualFilePath)
        , PermutationCount(InPermutationCount)
    {
    }

    void FShaderTypeBase::UpdateCode()
    {
        if (VirtualFilePath != "")
        {
            FileAbsolutePath = FPath::VirtualPathToAbsPath(VirtualFilePath);
            NILOU_LOG(Display, "Preprocessing {}", FileAbsolutePath.generic_string());
            std::string RawSourceCode = GetAssetLoader()->SyncOpenAndReadText(FileAbsolutePath.generic_string().c_str());
            PreprocessedCode = shader_preprocess::PreprocessInclude(RawSourceCode, FileAbsolutePath.parent_path().generic_string(), {});
            auto Filename = FPath::GetBaseFilename(FileAbsolutePath);
            std::ofstream DebugShaderCode(Filename);
            DebugShaderCode << PreprocessedCode;
            HashedName = FHashedName(Name+FileAbsolutePath.generic_string());
        }
    }
}