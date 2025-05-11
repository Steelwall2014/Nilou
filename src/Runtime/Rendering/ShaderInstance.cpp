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
                ShaderRHI = RHICreateVertexShader(Code, ShaderName);
                break;
            case EShaderStage::Pixel:
                ShaderRHI = RHICreatePixelShader(Code, ShaderName);
                break;
            case EShaderStage::Compute:
                ShaderRHI = RHICreateComputeShader(Code, ShaderName);
                break;
            default:
                break;
        }
        if (ShaderRHI)
        {
            std::string ErrorMessage;
            if (sr::ReflectShader(Code, ShaderStage, DescriptorSetLayouts, ErrorMessage))
            {
                ShaderRHI->Reflection = DescriptorSetLayouts;
            }
            else
            {
                {
                    std::ofstream out(ShaderName + ".glsl");
                    out << Code;
                }
                NILOU_LOG(Fatal, "Error occured during shader reflection of {}: \"{}\"", ShaderName, ErrorMessage);
            }
        }
    }

    void FShaderInstance::ReleaseRHI()
    {
        ShaderRHI = nullptr;
    }

}