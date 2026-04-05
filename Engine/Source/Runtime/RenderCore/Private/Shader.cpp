#include "DynamicRHI.h"
#include "RenderGraph.h"
#include "Shader.h"
// #include "Shadinclude.h"
#include "Shader.h"
#include "NObject/ObjectMacros.h"
#include "ShaderMap.h"
#include "Misc/Paths.h"
#include "ShaderPreprocess.h"
#include "Misc/FileHelper.h"

namespace fs = std::filesystem;

namespace nilou {


    /*========================Implement FShader::StaticType==========================*/
    FShaderType FShader::StaticType(
        "FShader", "", "", 
        EShaderFrequency::None,
        EShaderMetaType::None,
        FShader::ShouldCompilePermutation, 
        FShader::ModifyCompilationEnvironment,
        FShader::FPermutationDomain::PermutationCount);
    FShaderType *FShader::GetType() const
    {
        return &StaticType;
    }
    /*========================Implement FShader::StaticType==========================*/

    /*========================Implement FGlobalShader::StaticType==========================*/
	IMPLEMENT_SHADER_TYPE(FGlobalShader, "", "", EShaderFrequency::None)
    /*========================Implement FGlobalShader::StaticType==========================*/

    /*========================Implement FMaterialShader::StaticType==========================*/
	IMPLEMENT_SHADER_TYPE(FMaterialShader, "", "", EShaderFrequency::None)
    /*========================Implement FMaterialShader::StaticType==========================*/

	TShaderMap<FShaderPermutationParameters> GlobalShaders;
    void AddGlobalShader(const FShaderPermutationParameters &Parameters, RHIShaderRef ShaderRHI, bool overlap)
    {
        GlobalShaders.AddShader(ShaderRHI, Parameters);
    }

    RHIShader *GetGlobalShader(const FShaderPermutationParameters &Parameters)
    {
        return GlobalShaders.GetShader(Parameters);
    }

    std::vector<FShaderType *> &GetAllShaderTypes()
    {
    	static std::vector<FShaderType *> GAllShaderTypes;
        return GAllShaderTypes;
    }

    std::vector<FVertexFactoryType *> &GetAllVertexFactoryTypes()
    {
        static std::vector<FVertexFactoryType *> GAllVertexFactoryTypes;
        return GAllVertexFactoryTypes;
    }

    std::string RemovePrefixF(const std::string& Name)
    {
        if (Name.starts_with("F"))
        {
            return Name.substr(1);
        }
        return Name;
    }

    FShaderTypeBase::FShaderTypeBase(const std::string &InClassName, const std::string &InFilePath, const std::string &InEntryPointName, int32 InPermutationCount)
        : Name(RemovePrefixF(InClassName))
        , FileAbsolutePath(FPaths::EngineDir() + InFilePath)
        , EntryPointName(InEntryPointName)
        , PermutationCount(InPermutationCount)
    {
        if (fs::exists(FileAbsolutePath))
        {
            NILOU_LOG(Display, "Preprocessing {}", FileAbsolutePath.generic_string());
            std::string RawSourceCode;
            FFileHelper::LoadFileToString(RawSourceCode, FileAbsolutePath.generic_string());
            PreprocessedCode = shader_preprocess::PreprocessInclude(RawSourceCode, FileAbsolutePath.parent_path().generic_string(), {});
            HashedName = FHashedName(Name+FileAbsolutePath.generic_string());
        }
        else
        {
            if (InClassName != "FShader" && InClassName != "FGlobalShader" && InClassName != "FMaterialShader" && InClassName != "FVertexFactory")
            {
                NILOU_LOG(Error, "Shader file not found: {}", FileAbsolutePath.generic_string());
            }
        }
    }

    std::vector<FGraphicsPipeline*>& GetAllGraphicsPipelines()
    {
        static std::vector<FGraphicsPipeline*> GraphicsPipelines;
        return GraphicsPipelines;
    }

    FGraphicsPipeline::FGraphicsPipeline(
        const std::string& InPipelineName,
        FShaderType* InVertexShaderType,
        FShaderType* InPixelShaderType,
        std::function<bool(const FGraphicsPipelinePermutationParameters&)> InShouldCompilePermutation,
        std::function<void(const FGraphicsPipelinePermutationParameters&, FShaderCompilerEnvironment&)> InModifyCompilationEnvironment,
        int32 InPermutationCount)
        : Name(RemovePrefixF(InPipelineName))
        , HashedName(FHashedName(RemovePrefixF(InPipelineName)))
        , PermutationCount(InPermutationCount)
        , VertexShaderType(InVertexShaderType)
        , PixelShaderType(InPixelShaderType)
        , ShouldCompilePermutation(InShouldCompilePermutation)
        , ModifyCompilationEnvironment(InModifyCompilationEnvironment)
    {
        GetAllGraphicsPipelines().push_back(this);
    }

    /*========================Global Graphics Pipeline Map==========================*/
    struct FGlobalGraphicsPipelineMap
    {
        std::unordered_map<FHashedName, std::vector<RHIGraphicsPipelineShaders>> Pipelines;

        void Add(
            const FGraphicsPipelinePermutationParameters& Params,
            RHIVertexShaderRef VSShader,
            RHIPixelShaderRef PSShader,
            Slang::ComPtr<slang::ISession> SlangSession,
            Slang::ComPtr<slang::IComponentType> SlangComponent)
        {
            const FHashedName Key = Params.Type->HashedName;
            if (Pipelines.find(Key) == Pipelines.end())
            {
                Pipelines[Key] = std::vector<RHIGraphicsPipelineShaders>(Params.Type->PermutationCount);
            }
            Pipelines[Key][Params.PermutationId].VertexShader = VSShader;
            Pipelines[Key][Params.PermutationId].PixelShader  = PSShader;
            Pipelines[Key][Params.PermutationId].SlangSession  = std::move(SlangSession);
            Pipelines[Key][Params.PermutationId].SlangComponent = std::move(SlangComponent);
        }

        RHIGraphicsPipelineShaders* Get(const FGraphicsPipelinePermutationParameters& Params)
        {
            const FHashedName Key = Params.Type->HashedName;
            auto iter = Pipelines.find(Key);
            if (iter != Pipelines.end())
            {
                return &iter->second[Params.PermutationId];
            }
            return nullptr;
        }
    };

    static FGlobalGraphicsPipelineMap GGlobalGraphicsPipelines;

    void AddGlobalGraphicsPipeline(
        const FGraphicsPipelinePermutationParameters& Params,
        RHIVertexShaderRef VSShader,
        RHIPixelShaderRef PSShader,
        Slang::ComPtr<slang::ISession> SlangSession,
        Slang::ComPtr<slang::IComponentType> SlangComponent)
    {
        GGlobalGraphicsPipelines.Add(Params, VSShader, PSShader, std::move(SlangSession), std::move(SlangComponent));
    }

    RHIGraphicsPipelineShaders* GetGlobalGraphicsPipeline(const FGraphicsPipelinePermutationParameters& Params)
    {
        return GGlobalGraphicsPipelines.Get(Params);
    }
    /*========================Global Graphics Pipeline Map==========================*/
}