#pragma once
#include "Shader.h"
#include "Templates/ObjectMacros.h"
#include "DefferedShadingSceneRenderer.h"

namespace nilou {
    DECLARE_GLOBAL_SHADER(FLightingPassVS)
    class FLightingPassPS : public FGlobalShader
	{
	public:
		DECLARE_SHADER_TYPE()
        class FDimensionFrustumCount : SHADER_PERMUTATION_SPARSE_INT("FrustumCount", 1, 6, CASCADED_SHADOWMAP_SPLIT_COUNT);
        using FPermutationDomain = TShaderPermutationDomain<FDimensionFrustumCount>;
        static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameter, FShaderCompilerEnvironment& Environment);
	};
}