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

namespace nilou {


    /*========================Implement FShader::StaticType==========================*/
	// FShaderType FShader::StaticType; 
	// FShaderType* FShader::GetType() const { return &StaticType; }
	IMPLEMENT_SHADER_TYPE(FShader, "", EShaderFrequency::SF_None, None)
    /*========================Implement FShader::StaticType==========================*/

    /*========================Implement FGlobalShader::StaticType==========================*/
	// FShaderType FGlobalShader::StaticType( 
	// 	"FGlobalShader", 
	// 	"", 
	// 	EShaderFrequency::SF_None,
	// 	EShaderMetaType::SMT_Global,
	// 	FGlobalShader::ShouldCompilePermutation,
	// 	FGlobalShader::ModifyCompilationEnvironment,
	// 	FGlobalShader::FPermutationDomain::PermutationCount
	// 	); 
	// FShaderType* FGlobalShader::GetType() const { return &StaticType; }
	IMPLEMENT_SHADER_TYPE(FGlobalShader, "", EShaderFrequency::SF_None, Global)
    /*========================Implement FGlobalShader::StaticType==========================*/

    /*========================Implement FMaterialShader::StaticType==========================*/
	// FShaderType FMaterialShader::StaticType( 
	// 	"FMaterialShader", 
	// 	"", 
	// 	EShaderFrequency::SF_None,
	// 	EShaderMetaType::SMT_Material,
	// 	FMaterialShader::ShouldCompilePermutation,
	// 	FMaterialShader::ModifyCompilationEnvironment,
	// 	FMaterialShader::FPermutationDomain::PermutationCount
	// 	); 
	// FShaderType* FMaterialShader::GetType() const { return &StaticType; }
	IMPLEMENT_SHADER_TYPE(FMaterialShader, "", EShaderFrequency::SF_None, Material)
    /*========================Implement FMaterialShader::StaticType==========================*/

	TShaderMap<FShaderPermutationParameters> GlobalShaders;
    void AddGlobalShader(const FShaderPermutationParameters &Parameters, std::shared_ptr<FShaderInstance> ShaderRHI, bool overlap)
    {
        GlobalShaders.AddShader(ShaderRHI, Parameters);
    }

    FShaderInstance *GetGlobalShader(const FShaderPermutationParameters &Parameters)
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

    FShaderTypeBase::FShaderTypeBase(const std::string &InClassName, const std::string &InVirtualFilePath, int32 InPermutationCount)
        : Name(InClassName)
        , VirtualFilePath(InVirtualFilePath)
        , PermutationCount(InPermutationCount)
    {
    }

    void FShaderTypeBase::UpdateCode()
    {
        if (VirtualFilePath != "")
        {
            FileAbsolutePath = FPaths::EngineDir() + "/" + VirtualFilePath;
            NILOU_LOG(Display, "Preprocessing {}", FileAbsolutePath.generic_string());
            std::string RawSourceCode;
            FFileHelper::LoadFileToString(RawSourceCode, FileAbsolutePath.generic_string());
            PreprocessedCode = shader_preprocess::PreprocessInclude(RawSourceCode, FileAbsolutePath.parent_path().generic_string(), {});
            std::string Filename = FPaths::GetBaseFilename(FileAbsolutePath.generic_string());
            FFileHelper::SaveStringToFile(PreprocessedCode, Filename);
            HashedName = FHashedName(Name+FileAbsolutePath.generic_string());
        }
    }
}