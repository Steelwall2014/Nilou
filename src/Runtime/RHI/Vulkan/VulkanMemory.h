#pragma once

#include <vulkan/vulkan.h>
#include "Platform.h"

namespace nilou {

class FVulkanMemoryManager
{
public:
    FVulkanMemoryManager(VkDevice InDevice, VkPhysicalDevice InPhysDevice)
        : Device(InDevice)
        , PhysDevice(InPhysDevice)
    { }
    VkDevice Device;
    VkPhysicalDevice PhysDevice;
    void AllocateBufferMemory(VkDeviceMemory* Memory, VkBuffer Buffer, VkMemoryPropertyFlags MemPropertyFlags);
    void AllocateImageMemory(VkDeviceMemory* Memory, VkImage Image, VkMemoryPropertyFlags MemPropertyFlags);
    void FreeMemory(VkDeviceMemory Memory);

private:
    uint32 findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);
};

}