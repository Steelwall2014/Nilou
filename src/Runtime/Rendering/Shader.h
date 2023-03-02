#pragma once
#include <algorithm>
#include <filesystem>
#include <functional>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Common/AssertionMacros.h"
#include "RHIResources.h"
#include "ShaderParameter.h"
#include "Templates/ObjectMacros.h"
// #include "VertexFactory.h"
#include "ShaderType.h"
#include "ShaderPermutation.h"

namespace nilou {

    template<EShaderFrequency ShaderFrequency, bool IsMaterialShader>
    struct TShaderFrequencyAssertHelper
    { };
    template<EShaderFrequency ShaderFrequency>
    struct TShaderFrequencyAssertHelper<ShaderFrequency, false>
    { };
    template<EShaderFrequency ShaderFrequency>
    struct TShaderFrequencyAssertHelper<ShaderFrequency, true>
    { 
        static_assert(ShaderFrequency != EShaderFrequency::SF_Compute, "If the shader is derived from FMaterialShader, the ShaderFrequency MUST NOT be SF_Compute. ");
    };

    class FShader
    {

    /*==========FShaderType Interface===============*/
    public: 
        static FShaderType StaticType; 
        virtual FShaderType* GetType() const;
    /*==========FShaderType Interface===============*/

    public:
        FShader() { }

        using FPermutationDomain = FShaderPermutationNone;
        // using FPermutationParameters = FShaderPermutationParameters;

        /** Can be overridden by FShader subclasses to modify their compile environment just before compilation occurs. */
        static void ModifyCompilationEnvironment(const FShaderPermutationParameters&, FShaderCompilerEnvironment&) {}

        /** Can be overridden by FShader subclasses to determine whether a specific permutation should be compiled. */
        static bool ShouldCompilePermutation(const FShaderPermutationParameters&) { return true; }
        
    };

    class FMaterialShader : public FShader
    {
        DECLARE_SHADER_TYPE()
    };

    class FGlobalShader : public FShader
    {
        DECLARE_SHADER_TYPE()
    };
}