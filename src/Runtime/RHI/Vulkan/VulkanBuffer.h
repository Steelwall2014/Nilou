#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"

namespace nilou {

class VulkanBuffer : public RHIBuffer
{
public:
    VulkanBuffer(uint32 InStride, uint32 InSize, EBufferUsageFlags InUsage)
        : RHIBuffer(InStride, InSize, InUsage)
    { }
    virtual ~VulkanBuffer();
    VkBuffer Buffer;
    VkDeviceMemory Memory;
};
using VulkanBufferRef = std::shared_ptr<VulkanBuffer>;

class VulkanUniformBuffer : public RHIUniformBuffer
{
public:
    VulkanUniformBuffer(uint32 InSize, EUniformBufferUsage InUsage)
        : RHIUniformBuffer(InSize, InUsage)
    { }
    virtual ~VulkanUniformBuffer();
    VkBuffer Buffer;
    VkDeviceMemory Memory;
};
using VulkanUniformBufferRef = std::shared_ptr<VulkanUniformBuffer>;

}