#pragma once
#include "Shader.h"
#include "Templates/ObjectMacros.h"
#include "DefferedShadingSceneRenderer.h"

namespace nilou {
    // DECLARE_MATERIAL_SHADER(FShadowDepthVS)
    class FShadowDepthVS : public FMaterialShader
	{
	public:
		DECLARE_SHADER_TYPE()
        class FDimensionFrustumCount : SHADER_PERMUTATION_SPARSE_INT("FrustumCount", 1, 6, CASCADED_SHADOWMAP_SPLIT_COUNT);
        using FPermutationDomain = TShaderPermutationDomain<FDimensionFrustumCount>;
        static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameter, FShaderCompilerEnvironment& Environment);
	};
    DECLARE_GLOBAL_SHADER(FShadowDepthPS)
}