#include "Common/AssertionMacros.h"
#include "DynamicRHI.h"
#include "GameStatics.h"
#include "ShaderType.h"
// #include "Shadinclude.h"
#include "Shader.h"
#include "Common/AssetLoader.h"
#include "Templates/ObjectMacros.h"

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
}