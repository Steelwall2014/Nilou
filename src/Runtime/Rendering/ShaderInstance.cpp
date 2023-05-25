#include <glslang/Public/ResourceLimits.h>
#include <glslang/MachineIndependent/localintermediate.h>

#include "ShaderInstance.h"
#include "DynamicRHI.h"
#include "Common/Log.h"

namespace nilou {

    void FShaderInstance::InitRHI()
    {
        FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
        const char *code_c_str = Code.c_str();
        switch (PipelineStage) 
        {
            case EPipelineStage::PS_Vertex:
                ShaderRHI = RHICmdList->RHICreateVertexShader(Code);
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangVertex);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangVertex,  glslang::EShClientNone, 0);
                break;
            case EPipelineStage::PS_Pixel:
                ShaderRHI = RHICmdList->RHICreatePixelShader(Code);
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangFragment);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangFragment,  glslang::EShClientNone, 0);
                break;
            case EPipelineStage::PS_Compute:
                ShaderRHI = RHICmdList->RHICreateComputeShader(Code);
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangCompute);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangCompute,  glslang::EShClientNone, 0);
                break;
        }
        ShaderRHI->ShaderGlsl = ShaderGlsl.get();
        ShaderGlsl->setEnvClient(glslang::EShClientNone, glslang::EShTargetClientVersion(0));
        ShaderGlsl->setEnvTarget(glslang::EShTargetNone, glslang::EShTargetLanguageVersion(0));
        ShaderGlsl->setStrings(&code_c_str, 1);
        std::string preprocess;
        glslang::TShader::ForbidIncluder includer;
        bool res = ShaderGlsl->preprocess(GetResources(), 460, EProfile::ECoreProfile, false, false, EShMsgDefault, &preprocess, includer);
        res &= ShaderGlsl->parse(GetResources(), 460, false, EShMsgDefault);
        if (!res)
        {
            std::string info = ShaderGlsl->getInfoLog();
            std::string debuginfo = ShaderGlsl->getInfoDebugLog();
            NILOU_LOG(Error, "Shader parse error: {}\n{}\n{}", 
                ShaderName, 
                info,
                debuginfo);
            ShaderGlsl == nullptr;
        }
    }

    void FShaderInstance::ReleaseRHI()
    {
        ShaderRHI = nullptr;
    }

}