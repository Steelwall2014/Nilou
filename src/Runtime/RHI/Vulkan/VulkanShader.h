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
    TVulkanShader(VkDevice InDevice, shaderc_compilation_result_t InShaderSpirv) : BaseType(InShaderSpirv), Device(InDevice) { }

    virtual bool Success() override
    {
        return Module != nullptr;
    }

    virtual void ReleaseRHI() override
    {
        BaseType::ReleaseRHI();
        if (Module != VK_NULL_HANDLE)
            vkDestroyShaderModule(Device, Module, nullptr);
    }

    ~TVulkanShader() { ReleaseRHI(); }

private:

    VkDevice Device;

    VkShaderModule Module;
};

using VulkanVertexShader = TVulkanShader<RHIVertexShader>;
using VulkanPixelShader = TVulkanShader<RHIPixelShader>;
using VulkanComputeShader = TVulkanShader<RHIComputeShader>;

using VulkanVertexShaderRef = std::shared_ptr<VulkanVertexShader>;
using VulkanPixelShaderRef = std::shared_ptr<VulkanPixelShader>;
using VulkanComputeShaderRef = std::shared_ptr<VulkanComputeShader>;

}