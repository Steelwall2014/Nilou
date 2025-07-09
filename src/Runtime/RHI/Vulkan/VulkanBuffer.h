#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "Common/EnumClassFlags.h"

namespace nilou {

class VulkanDevice;
class VulkanBuffer;

class FVulkanStagingManager
{
public:
    FVulkanStagingManager(VkDevice InDevice)
        : Device(InDevice)
    {
    }

    void Deinit();

    RHIBuffer* AcquireBuffer(uint32 Size, VkBufferUsageFlags InUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VkMemoryPropertyFlags InMemoryReadFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Sets pointer to nullptr
    void ReleaseBuffer(RHIBuffer*& StagingBuffer);

protected:

    std::mutex StagingLock;
    
    std::vector<RHIBufferRef> UsedStagingBuffers;
    std::vector<RHIBufferRef> FreeStagingBuffers;

    VkDevice Device = VK_NULL_HANDLE;

    uint64 UsedMemory = 0;
    uint64 PeakUsedMemory = 0;

    friend class FVulkanDynamicRHI;
};

class VulkanBuffer : public RHIBuffer
{
public:
    VulkanBuffer(VulkanDevice* InDevice, uint32 InStride, uint32 InSize, EBufferUsageFlags InUsage)
        : RHIBuffer(InStride, InSize, InUsage)
        , Device(InDevice)
    { 
    }

    virtual ~VulkanBuffer();

    VkBuffer Handle{};
    VkDeviceMemory Memory{};
    VulkanDevice* Device;

    VkBufferUsageFlags UsageFlags;
    VkMemoryPropertyFlags MemoryReadFlags;
};
using VulkanBufferRef = TRefCountPtr<VulkanBuffer>;

inline VulkanBuffer* ResourceCast(RHIBuffer* Buffer)
{
    return static_cast<VulkanBuffer*>(Buffer);
}

}