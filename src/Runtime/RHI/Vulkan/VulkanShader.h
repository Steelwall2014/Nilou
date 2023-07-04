#pragma once
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>
#include "RHIResources.h"

namespace nilou {

template<typename BaseType>
class TVulkanShader : public BaseType
{
public:
    friend class FVulkanDynamicRHI;
    TVulkanShader(VkDevice InDevice) : Device(InDevice) { }

    virtual bool Success() override
    {
        return Module != nullptr;
    }

    virtual void ReleaseRHI() override
    {
        if (Module != VK_NULL_HANDLE)
            vkDestroyShaderModule(Device, Module, nullptr);
        if (ShadercResult != nullptr)
            shaderc_result_release(ShadercResult);
    }

private:

    VkDevice Device;

    VkShaderModule Module;

    shaderc_compilation_result* ShadercResult;
};

using VulkanVertexShader = TVulkanShader<RHIVertexShader>;
using VulkanPixelShader = TVulkanShader<RHIPixelShader>;
using VulkanComputeShader = TVulkanShader<RHIComputeShader>;

using VulkanVertexShaderRef = std::shared_ptr<VulkanVertexShader>;
using VulkanPixelShaderRef = std::shared_ptr<VulkanPixelShader>;
using VulkanComputeShaderRef = std::shared_ptr<VulkanComputeShader>;

}