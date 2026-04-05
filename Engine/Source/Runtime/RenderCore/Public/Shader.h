#pragma once
#include <algorithm>
#include <filesystem>
#include <functional>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "RHIResources.h"
#include "ShaderParameter.h"
#include "NObject/ObjectMacros.h"
// #include "VertexFactory.h"
#include "ShaderPermutation.h"

namespace nilou {


    class RHIDescriptorSetLayout;

    RENDERCORE_API std::vector<class FShaderType *> &GetAllShaderTypes();

    RENDERCORE_API std::vector<class FVertexFactoryType *> &GetAllVertexFactoryTypes();

    RENDERCORE_API std::vector<class FGraphicsPipeline*>& GetAllGraphicsPipelines();

    enum class EShaderFrequency
    {
        None,
        Vertex,
        Pixel,
        Compute
    };



    enum class EShaderMetaType
    {
        None,
        Global,
        Material
    };

    class RENDERCORE_API FShaderTypeBase
    {
    public:
        friend class FShaderCompiler;
        std::string Name;
        std::filesystem::path FileAbsolutePath;
        FHashedName HashedName;
        std::string PreprocessedCode;
        int32 PermutationCount;
        std::string EntryPointName;

        FShaderTypeBase() { }

        FShaderTypeBase(const std::string &InClassName, const std::string &InFileName, const std::string &InEntryPointName, int32 InPermutationCount);

        std::string GetName() const
        {
            return Name;
        }

        std::string GetEntryPointName() const
        {
            return EntryPointName;
        }

        FHashedName GetHashedFileName() const
        {
            return HashedName;
        }

    };

    struct FShaderPermutationParameters
    {
        class FShaderType *Type;
        int32 PermutationId;
        FShaderPermutationParameters(FShaderType *InType=nullptr, int32 InPermutationId=0)
            : Type(InType)
            , PermutationId(InPermutationId)
        {}
    };

    struct FShaderType : public FShaderTypeBase
    {
    public:
        friend class FShaderCompiler;
        EShaderFrequency ShaderFrequency;
        EShaderMetaType ShaderMetaType;

        FShaderType() {}
        
        FShaderType(
            const std::string &InShaderClassName, 
            const std::string &InShaderFileName, 
            const std::string &InEntryPointName,
            EShaderFrequency InShaderFrequency, 
            EShaderMetaType InShaderMetaType,
            std::function<bool(const FShaderPermutationParameters&)> InShouldCompilePermutation,
            std::function<void(const FShaderPermutationParameters&, FShaderCompilerEnvironment&)> InModifyCompilationEnvironment,
            int32 InPermutationCount
        )
            : FShaderTypeBase(InShaderClassName, InShaderFileName, InEntryPointName, InPermutationCount)
            , ShaderFrequency(InShaderFrequency)
            , ShaderMetaType(InShaderMetaType)
            , ShouldCompilePermutation(InShouldCompilePermutation)
            , ModifyCompilationEnvironment(InModifyCompilationEnvironment)
        { 
            GetAllShaderTypes().push_back(this);
        }

        std::function<bool(const FShaderPermutationParameters&)> ShouldCompilePermutation;
        std::function<void(const FShaderPermutationParameters&, FShaderCompilerEnvironment&)> ModifyCompilationEnvironment;
    };

    

    struct FVertexFactoryPermutationParameters
    {
        class FVertexFactoryType *Type;
        int32 PermutationId;
        FVertexFactoryPermutationParameters(FVertexFactoryType *InType=nullptr, int32 InPermutationId=0)
            : Type(InType)
            , PermutationId(InPermutationId)
        {}
    };

    struct FVertexFactoryType : public FShaderTypeBase
    {
    public:
        friend class FShaderCompiler;

        FVertexFactoryType() {}

        FVertexFactoryType(
            const std::string &InFactoryName, 
            const std::string &InShaderFileName,
            std::function<bool(const FVertexFactoryPermutationParameters&)> InShouldCompilePermutation,
            std::function<void(const FVertexFactoryPermutationParameters&, FShaderCompilerEnvironment&)> InModifyCompilationEnvironment,
            int32 InPermutationCount)
            : FShaderTypeBase(InFactoryName, InShaderFileName, "", InPermutationCount)
            , ShouldCompilePermutation(InShouldCompilePermutation)
            , ModifyCompilationEnvironment(InModifyCompilationEnvironment)
        {
            GetAllVertexFactoryTypes().push_back(this);
        }
        std::function<bool(const FVertexFactoryPermutationParameters&)> ShouldCompilePermutation;
        std::function<void(const FVertexFactoryPermutationParameters&, FShaderCompilerEnvironment&)> ModifyCompilationEnvironment;
    };

    struct FGraphicsPipelinePermutationParameters
    {
        class FGraphicsPipeline* Type;
        int32 PermutationId;
        FGraphicsPipelinePermutationParameters(FGraphicsPipeline* InType = nullptr, int32 InPermutationId = 0)
            : Type(InType)
            , PermutationId(InPermutationId)
        {}
    };

    class FGraphicsPipeline
    {
    public:
        friend class FShaderCompiler;

        std::string Name;
        FHashedName HashedName;
        int32 PermutationCount;
        FShaderType* VertexShaderType = nullptr;
        FShaderType* PixelShaderType = nullptr;

        std::function<bool(const FGraphicsPipelinePermutationParameters&)> ShouldCompilePermutation;
        std::function<void(const FGraphicsPipelinePermutationParameters&, FShaderCompilerEnvironment&)> ModifyCompilationEnvironment;

        FGraphicsPipeline() {}

        RENDERCORE_API FGraphicsPipeline(
            const std::string& InPipelineName,
            FShaderType* InVertexShaderType,
            FShaderType* InPixelShaderType,
            std::function<bool(const FGraphicsPipelinePermutationParameters&)> InShouldCompilePermutation,
            std::function<void(const FGraphicsPipelinePermutationParameters&, FShaderCompilerEnvironment&)> InModifyCompilationEnvironment,
            int32 InPermutationCount
        );
    };

    template<EShaderFrequency ShaderFrequency, bool IsMaterialShader>
    struct TShaderFrequencyAssertHelper
    { };
    template<EShaderFrequency ShaderFrequency>
    struct TShaderFrequencyAssertHelper<ShaderFrequency, false>
    { };
    template<EShaderFrequency ShaderFrequency>
    struct TShaderFrequencyAssertHelper<ShaderFrequency, true>
    { 
        static_assert(ShaderFrequency != EShaderFrequency::Compute, "If the shader is derived from FMaterialShader, the ShaderFrequency MUST NOT be SF_Compute. ");
    };

    class FShaderParameterBlock;
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

    RENDERCORE_API void AddGlobalShader(const FShaderPermutationParameters &Parameters, RHIShaderRef ShaderRHI, bool overlap=false);

    RENDERCORE_API RHIShader *GetGlobalShader(const FShaderPermutationParameters &Parameters);

    template <typename T>
    RHIShader *GetGlobalShader(int PermutationId=0)
    {
        FShaderPermutationParameters PermutationParameters(&T::StaticType, PermutationId);
        return GetGlobalShader(PermutationParameters);
    }

    RENDERCORE_API void AddGlobalGraphicsPipeline(
        const FGraphicsPipelinePermutationParameters& Params,
        RHIVertexShaderRef VSShader,
        RHIPixelShaderRef PSShader,
        Slang::ComPtr<slang::ISession> SlangSession,
        Slang::ComPtr<slang::IComponentType> SlangComponent);

    RENDERCORE_API RHIGraphicsPipelineShaders* GetGlobalGraphicsPipeline(const FGraphicsPipelinePermutationParameters& Params);

    template <typename T>
    RHIGraphicsPipelineShaders* GetGlobalGraphicsPipeline(int PermutationId = 0)
    {
        FGraphicsPipelinePermutationParameters Params(&T::StaticType, PermutationId);
        return GetGlobalGraphicsPipeline(Params);
    }
    
}