#pragma once

#include <filesystem>
#include <string>
#include <functional>

#include "HashedName.h"
#include "RHIResources.h"
#include "ShaderParameter.h"
#include "ShaderCompileEnvironment.h"
#include "Templates/RefCounting.h"

namespace nilou {

    class RHIDescriptorSetLayout;

    std::vector<class FShaderType *> &GetAllShaderTypes();

    std::vector<class FVertexFactoryType *> &GetAllVertexFactoryTypes();

    enum class EShaderFrequency
    {
        SF_None,
        SF_Vertex,
        SF_Pixel,
        SF_Compute
    };



    enum class EShaderMetaType
    {
        SMT_None,
        SMT_Global,
        SMT_Material
    };

    class FShaderParameterCode
    {
    public:
        std::string Name;
        EShaderParameterType ParameterType;
        std::string Code;
        bool operator<(const FShaderParameterCode &Other) const
        {
            return Name < Other.Name;
        }
        bool operator==(const FShaderParameterCode &Other) const
        {
            return Name == Other.Name;
        }
    };

    // struct FShaderCodeInitializer
    // {
    //     std::set<FShaderParameterCode> ParameterCodes;
    //     std::string SourceCodeBody;
    // };

	struct FNamedDescriptorSetLayout
	{
        FNamedDescriptorSetLayout() { }
        FNamedDescriptorSetLayout(RHIDescriptorSetLayout* InLayoutRHI, std::map<std::string, RHIDescriptorSetLayoutBinding> InNameToBinding)
            : LayoutRHI(InLayoutRHI)
            , NameToBinding(InNameToBinding)
        { }
		RHIDescriptorSetLayout* LayoutRHI = nullptr;
		std::map<std::string, RHIDescriptorSetLayoutBinding> NameToBinding;
        RHIDescriptorSetLayout* GetRHI() const { return LayoutRHI; }
	};

    class FShaderTypeBase
    {
    public:
        friend class FShaderCompiler;
        std::string Name;
        std::string VirtualFilePath;
        std::filesystem::path FileAbsolutePath;
        FHashedName HashedName;
        std::string PreprocessedCode;
        int32 PermutationCount;
        
        FShaderTypeBase() { }

        FShaderTypeBase(const std::string &InClassName, const std::string &InFileName, int32 InPermutationCount);

        void UpdateCode();

        FHashedName GetHashedFileName() const
        {
            return HashedName;
        }

        FNamedDescriptorSetLayout GetDescriptorSetLayout(int32 PermutationId, uint32 SetIndex) const
        {
            if (DescriptorSetLayouts.size() <= PermutationId)
            {
                return {};
            }
            auto &DescriptorSetLayoutMap = DescriptorSetLayouts[PermutationId];
            auto It = DescriptorSetLayoutMap.find(SetIndex);
            if (It != DescriptorSetLayoutMap.end())
            {
                return It->second;
            }
            return {};
        }

        // Descriptor set layouts of every set index for each permutation
        // For FMaterialShader and FVertexFactory, the map only contains one element, 
        // and its key can only be VERTEX_SHADER_SET_INDEX, PIXEL_SHADER_SET_INDEX or VERTEX_FACTORY_SET_INDEX.
        // For FGlobalShader, the map contains all set indices.
        std::vector<std::map<uint32, FNamedDescriptorSetLayout>> DescriptorSetLayouts;

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
            EShaderFrequency InShaderFrequency, 
            EShaderMetaType InShaderMetaType,
            std::function<bool(const FShaderPermutationParameters&)> InShouldCompilePermutation,
            std::function<void(const FShaderPermutationParameters&, FShaderCompilerEnvironment&)> InModifyCompilationEnvironment,
            int32 InPermutationCount
        )
            : FShaderTypeBase(InShaderClassName, InShaderFileName, InPermutationCount)
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
            : FShaderTypeBase(InFactoryName, InShaderFileName, InPermutationCount)
            , ShouldCompilePermutation(InShouldCompilePermutation)
            , ModifyCompilationEnvironment(InModifyCompilationEnvironment)
        {
            GetAllVertexFactoryTypes().push_back(this);
        }
        std::function<bool(const FVertexFactoryPermutationParameters&)> ShouldCompilePermutation;
        std::function<void(const FVertexFactoryPermutationParameters&, FShaderCompilerEnvironment&)> ModifyCompilationEnvironment;
    };
}