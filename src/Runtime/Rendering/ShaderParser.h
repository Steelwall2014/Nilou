#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <set>
#include <filesystem>

#include "ShaderParameter.h"

namespace nilou {

    struct FShaderParsedParameter
    {
        std::string Name;
        std::string Code;
        EShaderParameterType ParameterType;
    };

    class FShaderParser 
    {
    public:
        FShaderParser() { }
        FShaderParser(const std::string &InRawSourceCode, const std::filesystem::path &InFileParentDir)
            : Code(InRawSourceCode)
            , FileParentDir(InFileParentDir) {}
        std::string Parse();

    private:

        std::string ParseInclude();

        std::string ParseParameters();

        std::string Code;
        std::filesystem::path FileParentDir;
    };
}