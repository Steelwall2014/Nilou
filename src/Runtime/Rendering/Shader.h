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
#include "ShaderSegment.h"
#include "Templates/ObjectMacros.h"
// #include "VertexFactory.h"
#include "ShaderType.h"
#include "ShaderPermutation.h"

namespace nilou {

    // class FShaderParameterMapInfo
    // {
    // public:
    //     std::vector<FShaderParameterInfo> UniformBuffers;
    //     std::vector<FShaderParameterInfo> TextureSamplers;
    //     std::vector<FShaderParameterInfo> VertexAttributes;
    // };

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

    class FShader// : public FShaderSegment
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

        // bool IsPermutationValid(const std::set<std::string> &PossibleDefinitions, const std::set<std::string> &Permutation)
        // {
        //     if (!std::includes(Permutation.begin(), Permutation.end(), 
        //                        PossibleDefinitions.begin(), PossibleDefinitions.end()))
        //     {
        //         return false;
        //     }
        //     return true;
        // }

        // const RHIShader *GetOrCreateShaderByPermutation(const std::set<std::string> &Definitions) { return nullptr; }
    private:
        
    };

    class FMaterialShader : public FShader
    {
        DECLARE_SHADER_TYPE()
    };

    class FGlobalShaderResourceParameter
    {
        
    };

    class FGlobalShader : public FShader
    {
        DECLARE_SHADER_TYPE()
    };
}