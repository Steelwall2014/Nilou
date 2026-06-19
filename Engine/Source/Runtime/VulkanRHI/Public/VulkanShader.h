#pragma once
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>
#include "RHIResources.h"
#include "VulkanDevice.h"

namespace nilou {

template<typename BaseType>
class TVulkanShader : public BaseType
{
public:
    friend class FVulkanDynamicRHI;
    TVulkanShader(VulkanDevice* InDevice, const std::string& InName) : BaseType(InName), Device(InDevice) { }

    virtual bool Success() override
    {
        return Module != nullptr;
    }

    virtual void SetName(const std::string& NewName) override
    {
        BaseType::SetName(NewName);
#if VULKAN_ENABLE_DRAW_MARKERS
        if (!NewName.empty() && Module != VK_NULL_HANDLE)
        {
            Device->SetDebugUtilsObjectName(VK_OBJECT_TYPE_SHADER_MODULE, (uint64_t)Module, NewName.c_str());
        }
#endif
    }

    virtual void ReleaseRHI() override
    {
        BaseType::ReleaseRHI();
        if (Module != VK_NULL_HANDLE)
            vkDestroyShaderModule(Device->Handle, Module, nullptr);
    }

    ~TVulkanShader() { ReleaseRHI(); }

private:

    VulkanDevice* Device;

    VkShaderModule Module = VK_NULL_HANDLE;
};

using VulkanVertexShader = TVulkanShader<RHIVertexShader>;
using VulkanPixelShader = TVulkanShader<RHIPixelShader>;
using VulkanComputeShader = TVulkanShader<RHIComputeShader>;

using VulkanVertexShaderRef = TRefCountPtr<VulkanVertexShader>;
using VulkanPixelShaderRef = TRefCountPtr<VulkanPixelShader>;
using VulkanComputeShaderRef = TRefCountPtr<VulkanComputeShader>;

}