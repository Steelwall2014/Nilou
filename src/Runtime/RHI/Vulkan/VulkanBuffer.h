#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"

namespace nilou {

class VulkanBuffer : public RHIBuffer
{
public:
    enum
    {
        NUM_BUFFERS = 3
    };
    VulkanBuffer(uint32 InStride, uint32 InSize, EBufferUsageFlags InUsage)
        : RHIBuffer(InStride, InSize, InUsage)
    { }
    uint8 DynamicBufferIndex;
    VkBuffer Buffers[NUM_BUFFERS];
    VkDeviceMemory Memories[NUM_BUFFERS];
    virtual ~VulkanBuffer();
};
using VulkanBufferRef = std::shared_ptr<VulkanBuffer>;

class VulkanUniformBuffer : public RHIUniformBuffer
{
public:
    VulkanUniformBuffer(uint32 InSize, EUniformBufferUsage InUsage)
        : RHIUniformBuffer(InSize, InUsage)
    { }
    enum
    {
        NUM_BUFFERS = 3
    };
    virtual ~VulkanUniformBuffer();
    uint8 DynamicBufferIndex;
    VkBuffer Buffers[NUM_BUFFERS];
    VkDeviceMemory Memories[NUM_BUFFERS];
};
using VulkanUniformBufferRef = std::shared_ptr<VulkanUniformBuffer>;

}