#pragma once
#include "RHIResources.h"
#include "VulkanDynamicRHI.h"

namespace nilou {

template<typename BaseType>
class TVulkanShader : public BaseType
{
public:
    friend class FVulkanDynamicRHI;

    virtual bool Success() override
    {
        return Module != nullptr;
    }

    virtual void ReleaseRHI() override
    {
        FDynamicRHI::GetDynamicRHI()->RHIDestroyShader(this);
    }

private:

    class VkShaderModule_T* Module;

    class shaderc_compilation_result* ShadercResult;
};

using VulkanVertexShader = TVulkanShader<RHIVertexShader>;
using VulkanPixelShader = TVulkanShader<RHIPixelShader>;
using VulkanComputeShader = TVulkanShader<RHIComputeShader>;

using VulkanVertexShaderRef = std::shared_ptr<VulkanVertexShader>;
using VulkanPixelShaderRef = std::shared_ptr<VulkanPixelShader>;
using VulkanComputeShaderRef = std::shared_ptr<VulkanComputeShader>;

}