#include <regex>
#include <sstream>
#include <string>
#include "DynamicRHI.h"
#include "GameStatics.h"
#include "ShaderType.h"
#include "ShaderCompiler.h"
#include "Common/Log.h"
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
            FileAbsolutePath = fs::path(GetShaderAbsolutePathFromVirtualPath(VirtualFilePath));
            std::string RawSourceCode = g_pAssetLoader->SyncOpenAndReadText(FileAbsolutePath.generic_string().c_str());
            NILOU_LOG(Info, "Parsing " + FileAbsolutePath.generic_string());
            ParsedResult = FShaderParser(RawSourceCode, FileAbsolutePath.parent_path()).Parse();
            std::stringstream ParseInfo;
            ParseInfo << "Parse complete " + FileAbsolutePath.generic_string() << "\n"
                      << ParsedResult;
            NILOU_LOG(Info, ParseInfo.str());

            HashedName = FHashedName(Name+FileAbsolutePath.generic_string());

            bSourceCodeReaded = true;
        }
    }
}