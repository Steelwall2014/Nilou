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
        , FileAbsolutePath(InFilePath)
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
}