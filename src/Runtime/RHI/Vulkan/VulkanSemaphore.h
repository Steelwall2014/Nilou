#pragma once
#include <vulkan/vulkan.h>

namespace nilou {

class FVulkanSemaphore
{
public:
    FVulkanSemaphore(VkDevice InDevice)
        : Device(InDevice)
    {
        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(Device, &info, nullptr, &Handle);
    }
    ~FVulkanSemaphore()
    {
        vkDestroySemaphore(Device, Handle, nullptr);
    }
    VkSemaphore Handle;

private:
    VkDevice Device;
};

}
