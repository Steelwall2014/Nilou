#include "ShaderInstance.h"
#include "DynamicRHI.h"
#include "Common/Log.h"
#include "ShaderReflection.h"

namespace nilou {

    namespace sr = shader_reflection;

    void FShaderInstance::InitRHI()
    {
        switch (PipelineStage) 
        {
            case EPipelineStage::PS_Vertex:
                ShaderRHI = RHICreateVertexShader(Code);
                break;
            case EPipelineStage::PS_Pixel:
                ShaderRHI = RHICreatePixelShader(Code);
                break;
            case EPipelineStage::PS_Compute:
                ShaderRHI = RHICreateComputeShader(Code);
                break;
        }
        if (ShaderRHI)
            DescriptorSetLayouts = sr::ReflectShader(Code);
    }

    void FShaderInstance::ReleaseRHI()
    {
        ShaderRHI = nullptr;
    }

}