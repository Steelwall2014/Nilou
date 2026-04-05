#pragma once
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <unordered_map>

#include <slang-com-ptr.h>
#include <slang.h>

#include "HAL/Platform.h"
#include "ShaderMap.h"


namespace nilou {
    // void AddShaderSourceDirectoryMapping(const std::string& VirtualShaderDirectory, const std::string& RealShaderDirectory);
    // std::string GetShaderAbsolutePathFromVirtualPath(const std::string &VirtualFilePath);

    class FVertexFactoryPermutationParameters;
    class FShaderPermutationParameters;


    class RENDERCORE_API FShaderCompiler 
    {   
    public:
        static void CompileComputeShaders();

        static void CompileGlobalGraphicsPipelines();

        static void CompileMaterialShader(
            const std::string& MaterialName,
            const std::string& MaterialPath,
            FMaterialShaderMap* ShaderMap, 
            const std::string &MaterialParsedResult,
            const FShaderCompilerEnvironment &Environment);

    private:
        static void CompileComputeShader(
            const FShaderPermutationParameters &ShaderParams);

        static void CompileGlobalGraphicsPipeline(
            const FGraphicsPipelinePermutationParameters &PipelineParams);

        static void CompileMaterialGraphicsPipeline(
            const std::string& MaterialName,
            const std::string& MaterialPath,
            const FGraphicsPipelinePermutationParameters& PipelineParams,
            const FVertexFactoryPermutationParameters& VFParams,
            const FShaderCompilerEnvironment& Environment,
            FMaterialPipelineMap& OutPipelineMap);

    };

    RENDERCORE_API slang::IGlobalSession* GetSlangGlobalSession();
    
}