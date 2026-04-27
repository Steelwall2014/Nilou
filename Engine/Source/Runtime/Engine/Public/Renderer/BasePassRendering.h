#pragma once

#include "Shader.h"

namespace nilou {

    DECLARE_MATERIAL_SHADER(FBasePassVS)
    DECLARE_MATERIAL_SHADER(FBasePassPS)

    class FBasePassPipeline
    {
    public:
        using VertexShaderType = FBasePassVS;
        using PixelShaderType  = FBasePassPS;
        static FGraphicsPipeline StaticType;

        class FDimensionEnableIBL : SHADER_PERMUTATION_BOOL("ENABLE_IBL");
        using FPermutationDomain = TShaderPermutationDomain<FDimensionEnableIBL>;

        static bool ShouldCompilePermutation(const FGraphicsPipelinePermutationParameters&)
        {
            return true;
        }

        static void ModifyCompilationEnvironment(const FGraphicsPipelinePermutationParameters& Params, FShaderCompilerEnvironment& Environment)
        {
            FPermutationDomain Domain(Params.PermutationId);
            Domain.ModifyCompilationEnvironment(Environment);
        }
    };

}
