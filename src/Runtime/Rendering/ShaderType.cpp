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

namespace fs = std::filesystem;

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

    void FShaderTypeBase::ReadSourceCode()
    {
        if (VirtualFilePath != "" && bSourceCodeReaded == false)
        {
            bool a = VirtualFilePath == "/Shaders/VirtualHeightfieldMesh/VHM_create_nodelist.comp";
            FileAbsolutePath = fs::path(GetShaderAbsolutePathFromVirtualPath(VirtualFilePath));
            std::string RawSourceCode = GetAssetLoader()->SyncOpenAndReadText(FileAbsolutePath.generic_string().c_str());
            NILOU_LOG(Info, "Parsing {}", FileAbsolutePath.generic_string());
            PreprocessedCode = FShaderParser(RawSourceCode, FileAbsolutePath.parent_path()).Parse();

            HashedName = FHashedName(Name+FileAbsolutePath.generic_string());

            bSourceCodeReaded = true;
        }
    }
}