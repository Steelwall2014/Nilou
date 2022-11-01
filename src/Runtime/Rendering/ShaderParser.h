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
    
    struct FShaderParserResult
    {
        std::vector<FShaderParsedParameter> ParsedParameters;
        std::string MainCode;
    };

    inline std::ostream &operator<<(std::ostream &out, const FShaderParserResult ParsedResult)
    {
        out << "Parsed Parameters: \n";
        for (const FShaderParsedParameter &Parameter : ParsedResult.ParsedParameters)
        {
            out << "*********************\n";
            out << "Name: " << Parameter.Name << "\n";
            out << "Parameter type: ";
            switch (Parameter.ParameterType) {
            case EShaderParameterType::SPT_Sampler:
                out << "SPT_Sampler\n";
                break;
            case EShaderParameterType::SPT_ShaderStructureBuffer:
                out << "SPT_ShaderStructureBuffer\n";
                break;
            case EShaderParameterType::SPT_UniformBuffer:
                out << "SPT_UniformBuffer\n";
                break;
            }
            out << "Code: " << Parameter.Code << "\n";
        }
        if (!ParsedResult.ParsedParameters.empty())
            out << "*********************\n";
        return out;
    }

    class FShaderParser 
    {
    public:
        FShaderParser() { }
        FShaderParser(const std::string &InRawSourceCode, const std::filesystem::path &InFileParentDir)
            : Code(InRawSourceCode)
            , FileParentDir(InFileParentDir) {}
        FShaderParserResult Parse();

    private:

        std::string ParseInclude();

        std::string ParseParameters();

        std::string Code;
        std::filesystem::path FileParentDir;
        FShaderParserResult ParsedResult;
    };
}