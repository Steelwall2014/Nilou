#pragma once
#include "Shader.h"
#include "Templates/ObjectMacros.h"

namespace nilou {

    class FLightingPassVS : public FGlobalShader
    {
        DECLARE_SHADER_TYPE()
    };

    class FLightingPassPS : public FGlobalShader
    {
        DECLARE_SHADER_TYPE()
    public:

        // class FDimentionLightNum : SHADER_PERMUTATION_RANGE_INT("STATIC_LIGHT_NUM", 1, 10);

        // using FPermutationDomain = TShaderPermutationDomain<FDimentionLightNum>;
    };
}