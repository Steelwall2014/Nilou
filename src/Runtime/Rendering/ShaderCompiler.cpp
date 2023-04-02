#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <vector>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/MachineIndependent/localintermediate.h>

#include "DynamicRHI.h"
#include "ShaderCompiler.h"
#include "Common/AssertionMacros.h"
#include "Common/ContentManager.h"
#include "GameStatics.h"
#include "Material.h"
#include "Shader.h"
#include "ShaderInstance.h"
#include "ShaderMap.h"
#include "ShaderType.h"
// #include "Shadinclude.h"
#include "VertexFactory.h"
#include "Common/Log.h"

#ifdef NILOU_DEBUG
#include <fstream>
void Write(std::string filename, std::string code)
{
    std::ofstream out(filename);
    out << code;
}
#endif

namespace nilou {


    std::map<std::string, std::string> &GetGShaderSourceDirectoryMappings()
    {
        static std::map<std::string, std::string> GShaderSourceDirectoryMappings;
        return GShaderSourceDirectoryMappings;
    }

    void AddShaderSourceDirectoryMapping(const std::string& VirtualShaderDirectory, const std::string& RealShaderDirectory)
    {
        check(std::filesystem::exists(RealShaderDirectory));
        check(GetGShaderSourceDirectoryMappings().count(VirtualShaderDirectory) == 0);
        GetGShaderSourceDirectoryMappings()[VirtualShaderDirectory] = RealShaderDirectory;
    }

    std::string GetShaderAbsolutePathFromVirtualPath(const std::string &VirtualFilePath)
    {
        bool RealFilePathFound = false;
        std::filesystem::path RealFilePath;
        std::filesystem::path ParentVirtualDirectoryPath = std::filesystem::path(VirtualFilePath).parent_path();
        std::filesystem::path RelativeVirtualDirectoryPath = std::filesystem::path(VirtualFilePath).filename();

        while (!ParentVirtualDirectoryPath.empty() && ParentVirtualDirectoryPath.generic_string() != "/")
        {
            if (GetGShaderSourceDirectoryMappings().count(ParentVirtualDirectoryPath.generic_string()) != 0)
            {
                RealFilePath = 
                    std::filesystem::path(GetGShaderSourceDirectoryMappings()[ParentVirtualDirectoryPath.generic_string()]) / RelativeVirtualDirectoryPath;
                RealFilePathFound = true;
                break;
            }

            RelativeVirtualDirectoryPath = ParentVirtualDirectoryPath.filename() / RelativeVirtualDirectoryPath;
            ParentVirtualDirectoryPath = ParentVirtualDirectoryPath.parent_path();
        }
        if (!RealFilePathFound)
            std::cout << "[ERROR] Can't map virtual shader source path " << VirtualFilePath << std::endl;
        return RealFilePath.generic_string();
    }

    void AddUniformsToSStream(const std::set<FShaderParameterCode> &ParameterCodes, std::stringstream &Out)
    {
        for (auto &ParameterCode : ParameterCodes)
            Out << ParameterCode.Code << "\n";
    }

    std::stringstream &operator<<(std::stringstream &out, const std::set<FShaderParameterCode> &ParameterCodes)
    {
        for (auto &ParameterCode : ParameterCodes)
            out << ParameterCode.Code << "\n";
        return out;
    }

    template<int N>
    std::string ConcateShaderCodeAndParameters(
        /*std::set<FShaderParameterInfo> &OutShaderParameters, */
        std::array<const std::string*, N> PreprocessResults, 
        const FShaderCompilerEnvironment &Environment,
        FDynamicRHI *DynamicRHI)
    {
        std::stringstream stream;
        stream << "#version 460\n";
        if (DynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
            stream << "#define USING_OPENGL 1\n";

        for (auto &[key, value] : Environment.Definitions)
            stream << "#define " << key << " " << value << "\n";
        for (const std::string* Code : PreprocessResults)
        {
            stream << *Code;
        }

        return stream.str();
    }

    void FShaderCompiler::CompileGlobalShader(
        FDynamicRHI *DynamicRHI, 
        const FShaderPermutationParameters &ShaderParameter)
    {
        FShaderType *ShaderType = ShaderParameter.Type;
        
        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>();
        std::string code = ConcateShaderCodeAndParameters<1>(
            /*ShaderInstance->Parameters, */{&ShaderType->PreprocessedCode}, Environment, DynamicRHI);
        const char *code_c_str = code.c_str();
        ShaderInstance->ShaderName = ShaderType->Name;

        switch (ShaderType->ShaderFrequency) 
        {
            case EShaderFrequency::SF_Vertex:
                ShaderInstance->ShaderRHI = DynamicRHI->RHICreateVertexShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Vertex;
                ShaderInstance->ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangVertex);
                ShaderInstance->ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangVertex,  glslang::EShClientNone, 0);
                break;
            case EShaderFrequency::SF_Pixel:
                ShaderInstance->ShaderRHI = DynamicRHI->RHICreatePixelShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Pixel;
                ShaderInstance->ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangFragment);
                ShaderInstance->ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangFragment,  glslang::EShClientNone, 0);
                break;
            case EShaderFrequency::SF_Compute:
                ShaderInstance->ShaderRHI = DynamicRHI->RHICreateComputeShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Compute;
                ShaderInstance->ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangCompute);
                ShaderInstance->ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangCompute,  glslang::EShClientNone, 0);
                break;
        }
        
        ShaderInstance->ShaderGlsl->setEnvClient(glslang::EShClientNone, glslang::EShTargetClientVersion(0));
        ShaderInstance->ShaderGlsl->setEnvTarget(glslang::EShTargetNone, glslang::EShTargetLanguageVersion(0));
        ShaderInstance->ShaderGlsl->setStrings(&code_c_str, 1);
        std::string preprocess;
        glslang::TShader::ForbidIncluder includer;
        bool res = ShaderInstance->ShaderGlsl->preprocess(GetResources(), 460, EProfile::ECoreProfile, false, false, EShMsgDefault, &preprocess, includer);
        res &= ShaderInstance->ShaderGlsl->parse(GetResources(), 460, false, EShMsgDefault);
        if (!res)
        {
            std::string info = ShaderInstance->ShaderGlsl->getInfoLog();
            std::string debuginfo = ShaderInstance->ShaderGlsl->getInfoDebugLog();
            NILOU_LOG(Error, "Shader parse error: {}\n{}\n{}", 
                ShaderType->Name, 
                info,
                debuginfo);
            ShaderInstance->ShaderGlsl == nullptr;
        }
        GetContentManager()->AddGlobalShader(ShaderParameter, ShaderInstance);
    }

    void FShaderCompiler::CompileVertexMaterialShader(
        FDynamicRHI *DynamicRHI,
        FMaterial *Material, 
        const std::string &MaterialPreprocessedResult,
        const FVertexFactoryPermutationParameters &VertexFactoryParams,
        const FShaderPermutationParameters &ShaderParameter,
        TShaderMap<FVertexFactoryPermutationParameters, FShaderPermutationParameters> &OutShaderMap)
    {
        FVertexFactoryType *VertexFactoryType = VertexFactoryParams.Type;
        FShaderType *ShaderType = ShaderParameter.Type;

        VertexFactoryType->ReadSourceCode();

        // Material Vertex Shader
        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);
        VertexFactoryType->ModifyCompilationEnvironment(VertexFactoryParams, Environment);

        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>();
        std::string code = ConcateShaderCodeAndParameters<3>(
            {&MaterialPreprocessedResult, &VertexFactoryType->PreprocessedCode, &ShaderType->PreprocessedCode}, 
            Environment, DynamicRHI);
        const char *code_c_str = code.c_str();
        ShaderInstance->ShaderName = ShaderType->Name;

        ShaderInstance->ShaderRHI = DynamicRHI->RHICreateVertexShader(code.c_str());
        ShaderInstance->PipelineStage = EPipelineStage::PS_Vertex;

        ShaderInstance->ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangVertex);
        ShaderInstance->ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangVertex,  glslang::EShClientNone, 0);
        ShaderInstance->ShaderGlsl->setEnvClient(glslang::EShClientNone, glslang::EShTargetClientVersion(0));
        ShaderInstance->ShaderGlsl->setEnvTarget(glslang::EShTargetNone, glslang::EShTargetLanguageVersion(0));
        ShaderInstance->ShaderGlsl->setStrings(&code_c_str, 1);
        std::string preprocess;
        glslang::TShader::ForbidIncluder includer;
        bool res = ShaderInstance->ShaderGlsl->preprocess(GetResources(), 100, EProfile::ECoreProfile, false, false, EShMsgDefault, &preprocess, includer);
        res &= ShaderInstance->ShaderGlsl->parse(GetResources(), 100, false, EShMsgDefault);
        if (!res)
        {

            std::string info = ShaderInstance->ShaderGlsl->getInfoLog();
            std::string debuginfo = ShaderInstance->ShaderGlsl->getInfoDebugLog();
            NILOU_LOG(Error, "Shader parse error: {}\n{}\n{}", 
                ShaderType->Name, 
                info,
                debuginfo);
            ShaderInstance->ShaderGlsl == nullptr;
        }
        
        OutShaderMap.AddShader(ShaderInstance, VertexFactoryParams, ShaderParameter);
    }

    void FShaderCompiler::CompilePixelMaterialShader(
        FDynamicRHI *DynamicRHI, 
        FMaterial *Material, 
        const std::string& MaterialParsedResult,
        const FShaderPermutationParameters &ShaderParameter,
        TShaderMap<FShaderPermutationParameters> &OutShaderMap)
    {
        FShaderType *ShaderType = ShaderParameter.Type;

        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>();
        std::string code = ConcateShaderCodeAndParameters<2>(
            {&MaterialParsedResult, &ShaderType->PreprocessedCode}, 
            Environment, DynamicRHI);
        const char *code_c_str = code.c_str();
        ShaderInstance->ShaderName = ShaderType->Name;

        ShaderInstance->ShaderRHI = DynamicRHI->RHICreatePixelShader(code.c_str());
        ShaderInstance->PipelineStage = EPipelineStage::PS_Pixel;
        
        ShaderInstance->ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangFragment);
        ShaderInstance->ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangFragment,  glslang::EShClientNone, 0);
        ShaderInstance->ShaderGlsl->setEnvClient(glslang::EShClientNone, glslang::EShTargetClientVersion(0));
        ShaderInstance->ShaderGlsl->setEnvTarget(glslang::EShTargetNone, glslang::EShTargetLanguageVersion(0));
        ShaderInstance->ShaderGlsl->setStrings(&code_c_str, 1);
        std::string preprocess;
        glslang::TShader::ForbidIncluder includer;
        bool res = ShaderInstance->ShaderGlsl->preprocess(GetResources(), 100, EProfile::ECoreProfile, false, false, EShMsgDefault, &preprocess, includer);
        res &= ShaderInstance->ShaderGlsl->parse(GetResources(), 100, false, EShMsgDefault);
        if (!res)
        {

            std::string info = ShaderInstance->ShaderGlsl->getInfoLog();
            std::string debuginfo = ShaderInstance->ShaderGlsl->getInfoDebugLog();
            NILOU_LOG(Error, "Shader parse error: {}\n{}\n{}", 
                ShaderType->Name, 
                info,
                debuginfo);
            ShaderInstance->ShaderGlsl == nullptr;
        }

        OutShaderMap.AddShader(ShaderInstance, ShaderParameter);
    }

    template<typename Func, typename Filter>
    void ForEachShader(Func f, Filter filter)
    {
        std::vector<FShaderType *> &ShaderTypes = GetAllShaderTypes();
        for (FShaderType *ShaderType : ShaderTypes)
        {
            if (ShaderType->ShaderFrequency == EShaderFrequency::SF_None)
                continue;
            if (!filter(ShaderType))
                continue;
            ShaderType->ReadSourceCode();
            for (int32 PermutationId = 0; PermutationId < ShaderType->PermutationCount; PermutationId++)
            {
                FShaderPermutationParameters ShaderParameter(ShaderType, PermutationId);
                if (!ShaderType->ShouldCompilePermutation(ShaderParameter))
                    continue;
                f(ShaderParameter);
            }
        }

    }

    template<typename Func, typename Filter>
    void ForEachVertexFactory(Func f, Filter filter)
    {
        std::vector<FVertexFactoryType *> &VertexFactoryTypes = GetAllVertexFactoryTypes();
        for (FVertexFactoryType *VertexFactoryType : VertexFactoryTypes)
        {
            if (VertexFactoryType->Name == "FVertexFactory")
                continue;
            if (!filter(VertexFactoryType))
                continue;
            VertexFactoryType->ReadSourceCode();
            for (int32 VFPermutationId = 0; VFPermutationId < VertexFactoryType->PermutationCount; VFPermutationId++)
            {
                FVertexFactoryPermutationParameters VFParameters(VertexFactoryType, VFPermutationId);
                if (!VertexFactoryType->ShouldCompilePermutation(VFParameters))
                    continue;
                f(VFParameters);
            }
        }

    }

    void FShaderCompiler::CompileGlobalShaders(FDynamicRHI *DynamicRHI)
    {
        ForEachShader(
            [DynamicRHI](const FShaderPermutationParameters &ShaderParameter) 
            {
                CompileGlobalShader(DynamicRHI, ShaderParameter);
            },
            [](FShaderType *ShaderType) 
            {
                return ShaderType->ShaderMetaType == EShaderMetaType::SMT_Global;
            });
    }
    
    void FShaderCompiler::CompileMaterialShader(FMaterial *Material, 
        const std::string &MaterialParsedResult,FDynamicRHI *DynamicRHI)
    {
        ForEachShader(
            [Material, &MaterialParsedResult, DynamicRHI](const FShaderPermutationParameters &ShaderParameter) {   
                FShaderType *ShaderType = ShaderParameter.Type;             
                if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Vertex)
                {
                    ForEachVertexFactory(
                    [Material, &MaterialParsedResult, &ShaderParameter, DynamicRHI]
                    (const FVertexFactoryPermutationParameters &VFParameters) {
                        CompileVertexMaterialShader(
                            DynamicRHI, Material, MaterialParsedResult, VFParameters, ShaderParameter, 
                            Material->ShaderMap->VertexShaderMap);
                    }, 
                    [](FVertexFactoryType *) 
                    { return true; });
                }
                else if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Pixel)
                {
                    CompilePixelMaterialShader(
                        DynamicRHI, Material, MaterialParsedResult, ShaderParameter, 
                        Material->ShaderMap->PixelShaderMap);
                }
            },
            [](FShaderType *ShaderType) {
                return ShaderType->ShaderMetaType == EShaderMetaType::SMT_Material;
            });

    }
}