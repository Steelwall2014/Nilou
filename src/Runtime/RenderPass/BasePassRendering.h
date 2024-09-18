#pragma once

#include "Shader.h"
#include "Templates/ObjectMacros.h"
#include "VertexFactory.h"
#include "RenderPass.h"

namespace nilou {
    DECLARE_MATERIAL_SHADER(FBasePassVS)
    class FBasePassPS : public FMaterialShader
	{
	public:
		DECLARE_SHADER_TYPE()
        class FDimensionEnableReflectionProbe : SHADER_PERMUTATION_BOOL("ENABLE_REFLECTION_PROBE");
        using FPermutationDomain = TShaderPermutationDomain<FDimensionEnableReflectionProbe>;
        static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameter, FShaderCompilerEnvironment& Environment)
        {
            FPermutationDomain Domain(Parameter.PermutationId);
            Domain.ModifyCompilationEnvironment(Environment);
        }
	};

}