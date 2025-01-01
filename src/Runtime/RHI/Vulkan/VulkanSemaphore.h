#pragma once
#include <vulkan/vulkan.h>
#include "RHITransition.h"

namespace nilou {

class FVulkanSemaphore
{
public:
    FVulkanSemaphore()
        : Device(VK_NULL_HANDLE)
    {
    }
    FVulkanSemaphore(VkDevice InDevice)
        : Device(InDevice)
    {
        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(Device, &info, nullptr, &Handle);
    }
    ~FVulkanSemaphore()
    {
        if (Device && Handle)
            vkDestroySemaphore(Device, Handle, nullptr);
    }
    VkSemaphore Handle{};

    VkDevice Device{};
};

class VulkanSemaphore : public RHISemaphore 
{
public:
    VulkanSemaphore()
        : Device(VK_NULL_HANDLE)
    {
    }
    VulkanSemaphore(VkDevice InDevice)
        : Device(InDevice)
    {
        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(Device, &info, nullptr, &Handle);
    }
    ~VulkanSemaphore()
    {
        if (Device && Handle)
            vkDestroySemaphore(Device, Handle, nullptr);
    }
    VkSemaphore Handle{};

    VkDevice Device{};
};

inline VulkanSemaphore* ResourceCast(RHISemaphore* Semaphore)
{
    return static_cast<VulkanSemaphore*>(Semaphore);
}

}
