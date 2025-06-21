#include "ShaderInstance.h"
#include "DynamicRHI.h"
#include "Common/Log.h"
#include "ShaderReflection.h"

namespace nilou {

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
                Ncheckf(false, "Unsupported shader stage: {}", magic_enum::enum_name(ShaderStage));
        }
    }

    void FShaderInstance::ReleaseRHI()
    {
        ShaderRHI = nullptr;
    }

}