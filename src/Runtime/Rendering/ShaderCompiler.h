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

    std::string ProcessCodeInclude(const std::string &RawSourceCode, const std::filesystem::path &FileParentDir);

    std::string ProcessCodeShaderParams(const std::string &RawSourceCode, std::set<class FShaderParameterCode> &);

    class FDynamicRHI;
    class FVertexFactoryPermutationParameters;
    class FShaderPermutationParameters;


    class FShaderCompiler 
    {   
    public:
        static void CompileGlobalShaders();

        static void CompileMaterialShader(class FMaterial *Material);

    private:
        static void FShaderCompiler::CompileVertexMaterialShader(
            FDynamicRHI *RHICmdList,
            FMaterial *Material, 
            const FVertexFactoryPermutationParameters &VertexFactoryParams,
            const FShaderPermutationParameters &ShaderParams,
            TShaderMap<FVertexFactoryPermutationParameters, FShaderPermutationParameters> &OutShaderMap);
        static void CompilePixelMaterialShader(
            FDynamicRHI *RHICmdList,
            FMaterial *Material, 
            const FShaderPermutationParameters &ShaderParams,
            TShaderMap<FShaderPermutationParameters> &OutShaderMap);
        static void CompileGlobalShader(
            FDynamicRHI *RHICmdList, 
            const FShaderPermutationParameters &ShaderParams);

        /** Helper function for compiling material shaders */
        static void IterateOnMaterials(
            FDynamicRHI *RHICmdList, 
            const FShaderPermutationParameters &ShaderParams);

        /** Helper function for compiling vertex material shaders */
        static void FShaderCompiler::IterateOnVertexFactories(
            FDynamicRHI *RHICmdList, 
            FMaterial *Material,
            const FShaderPermutationParameters &ShaderParams);

    };
}