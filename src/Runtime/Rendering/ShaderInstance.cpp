#include "ShaderInstance.h"
#include "DynamicRHI.h"
#include "Common/Log.h"
#include "ShaderReflection.h"

namespace nilou {

    namespace sr = shader_reflection;

    void FShaderInstance::InitRHI()
    {
        switch (ShaderStage) 
        {
            case EShaderStage::Vertex:
                ShaderRHI = RHICreateVertexShader(Code);
                break;
            case EShaderStage::Pixel:
                ShaderRHI = RHICreatePixelShader(Code);
                break;
            case EShaderStage::Compute:
                ShaderRHI = RHICreateComputeShader(Code);
                break;
            default:
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