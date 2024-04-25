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
        glslang::EShClient client = glslang::EShClientOpenGL;
        glslang::EShTargetClientVersion version = glslang::EShTargetOpenGL_450;
        if (RHICmdList->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
        {
            client = glslang::EShClientVulkan;
            version = glslang::EShTargetVulkan_1_3;
        }
        switch (PipelineStage) 
        {
            case EPipelineStage::PS_Vertex:
                ShaderRHI = RHICmdList->RHICreateVertexShader(Code);
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangVertex);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangVertex,  client, 0);
                break;
            case EPipelineStage::PS_Pixel:
                ShaderRHI = RHICmdList->RHICreatePixelShader(Code);
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangFragment);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangFragment,  client, 0);
                break;
            case EPipelineStage::PS_Compute:
                ShaderRHI = RHICmdList->RHICreateComputeShader(Code);
                ShaderGlsl = std::make_unique<glslang::TShader>(EShLanguage::EShLangCompute);
                ShaderGlsl->setEnvInput(glslang::EShSourceGlsl , EShLanguage::EShLangCompute,  client, 0);
                break;
        }
        ShaderGlsl->setEnvClient(client, version);
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
            NILOU_LOG(Error, "Shader code: \n{}", Code);
            ShaderGlsl = nullptr;
            ShaderRHI = nullptr;
            return;
        }

        ShaderRHI->ShaderGlsl = ShaderGlsl.get();
    }

    void FShaderInstance::ReleaseRHI()
    {
        ShaderRHI = nullptr;
    }

}