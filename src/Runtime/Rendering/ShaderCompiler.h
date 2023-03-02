#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <set>
#include <filesystem>

#include "Platform.h"
#include "ShaderMap.h"

namespace nilou {
    void AddShaderSourceDirectoryMapping(const std::string& VirtualShaderDirectory, const std::string& RealShaderDirectory);
    std::string GetShaderAbsolutePathFromVirtualPath(const std::string &VirtualFilePath);

    class FDynamicRHI;
    class FVertexFactoryPermutationParameters;
    class FShaderPermutationParameters;


    class FShaderCompiler 
    {   
    public:
        static void CompileGlobalShaders(FDynamicRHI *DynamicRHI);

        static void CompileMaterialShader(class FMaterial *Material, 
            const std::string &MaterialParsedResult, FDynamicRHI *DynamicRHI);

    private:
        static void FShaderCompiler::CompileVertexMaterialShader(
            FDynamicRHI *DynamicRHI,
            FMaterial *Material, 
            const std::string &MaterialParsedResult,
            const FVertexFactoryPermutationParameters &VertexFactoryParams,
            const FShaderPermutationParameters &ShaderParams,
            TShaderMap<FVertexFactoryPermutationParameters, FShaderPermutationParameters> &OutShaderMap);
        static void CompilePixelMaterialShader(
            FDynamicRHI *DynamicRHI,
            FMaterial *Material, 
            const std::string &MaterialParsedResult,
            const FShaderPermutationParameters &ShaderParams,
            TShaderMap<FShaderPermutationParameters> &OutShaderMap);
        static void CompileGlobalShader(
            FDynamicRHI *DynamicRHI, 
            const FShaderPermutationParameters &ShaderParams);

        /** Helper function for compiling material shaders */
        static void IterateOnMaterials(
            FDynamicRHI *DynamicRHI, 
            const FShaderPermutationParameters &ShaderParams);

        /** Helper function for compiling vertex material shaders */
        static void FShaderCompiler::IterateOnVertexFactories(
            FDynamicRHI *DynamicRHI, 
            FMaterial *Material,
            const FShaderPermutationParameters &ShaderParams);

    };
}