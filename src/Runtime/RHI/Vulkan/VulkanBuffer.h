#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "Common/EnumClassFlags.h"

namespace nilou {

class VulkanDevice;

class VulkanBuffer : public RHIBuffer
{
public:
    VulkanBuffer(VulkanDevice* Device, uint32 InStride, uint32 InSize, EBufferUsageFlags InUsage)
        : RHIBuffer(InStride, InSize, InUsage)
    { 
    }

    VkBuffer Handle{};
    VkDeviceMemory Memory{};
    VulkanDevice* Device;
};
using VulkanBufferRef = TRefCountPtr<VulkanBuffer>;

inline VulkanBuffer* ResourceCast(RHIBuffer* Buffer)
{
    return static_cast<VulkanBuffer*>(Buffer);
}

}