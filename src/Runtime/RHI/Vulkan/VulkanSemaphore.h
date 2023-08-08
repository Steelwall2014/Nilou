#pragma once
#include <vulkan/vulkan.h>

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

}
