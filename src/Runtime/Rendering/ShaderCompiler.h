#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <set>
#include <filesystem>

#include "Platform.h"
#include "ShaderMap.h"

namespace nilou {
    // void AddShaderSourceDirectoryMapping(const std::string& VirtualShaderDirectory, const std::string& RealShaderDirectory);
    // std::string GetShaderAbsolutePathFromVirtualPath(const std::string &VirtualFilePath);

    class FVertexFactoryPermutationParameters;
    class FShaderPermutationParameters;


    class FShaderCompiler 
    {   
    public:
        static void CompileGlobalShaders();

        static void CompileMaterialShader(
            std::string MaterialName,
            FMaterialShaderMap* ShaderMap, 
            const std::string &MaterialParsedResult);

    private:
        static void CompileVertexMaterialShader(
            const std::string& MaterialName,
            const std::string &MaterialParsedResult,
            const FVertexFactoryPermutationParameters &VertexFactoryParams,
            const FShaderPermutationParameters &ShaderParams,
            TShaderMap<FVertexFactoryPermutationParameters, FShaderPermutationParameters> &OutShaderMap);
        static void CompilePixelMaterialShader(
            const std::string& MaterialName,
            const std::string &MaterialParsedResult,
            const FShaderPermutationParameters &ShaderParams,
            TShaderMap<FShaderPermutationParameters> &OutShaderMap);
        static void CompileGlobalShader(
            const FShaderPermutationParameters &ShaderParams);

    };
}