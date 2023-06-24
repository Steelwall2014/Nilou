#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "Common/EnumClassFlags.h"

namespace nilou {

class FVulkanCmdBuffer;

struct FStagingBuffer
{
    VkDevice Device;
    VkBuffer Buffer;
    VkDeviceMemory StagingBufferMemory;
    VkMemoryPropertyFlagBits MemoryReadFlags;
    uint32 BufferSize;
    void* MappedPointer;

    ~FStagingBuffer();

    void* GetMappedPointer();

    bool IsCoherent() const
    {
        return MemoryReadFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT != 0;
    }

    void FlushMappedMemory();
    void InvalidateMappedMemory();
};
using FStagingBufferRef = std::shared_ptr<FStagingBuffer>;

struct FPendingBufferLock
{
    FStagingBuffer* StagingBuffer;
    uint32 Offset;
    uint32 Size;
    EDataAccessFlag LockMode;
};

class FVulkanStagingManager
{
public:
    FVulkanStagingManager(VkDevice InDevice, class FVulkanDynamicRHI* InVulkanRHI) :
        PeakUsedMemory(0),
        UsedMemory(0),
        Device(InDevice),
        VulkanRHI(InVulkanRHI)
    {
    }

    FStagingBuffer* AcquireBuffer(uint32 Size, VkBufferUsageFlags InUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VkMemoryPropertyFlagBits InMemoryReadFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Sets pointer to nullptr
    void ReleaseBuffer(FVulkanCmdBuffer* CmdBuffer, FStagingBuffer*& StagingBuffer);

protected:
    friend class FMemoryManager;

    std::mutex StagingLock;

    std::vector<FStagingBufferRef> UsedStagingBuffers;

    struct FFreeEntry
    {
        FStagingBufferRef StagingBuffer;
        uint32 FrameNumber;
    };
    std::vector<FFreeEntry> FreeStagingBuffers;

    uint64 PeakUsedMemory;
    uint64 UsedMemory;

    VkDevice Device;
    FVulkanDynamicRHI* VulkanRHI;
};

class VulkanBuffer : public RHIBuffer
{
public:
	enum class ELockStatus : uint8
	{
		Unlocked,
		Locked,
		PersistentMapping,
	} LockStatus;
    enum
    {
        NUM_BUFFERS = 3
    };
    VulkanBuffer(uint32 InStride, uint32 InSize, EBufferUsageFlags InUsage)
        : RHIBuffer(InStride, InSize, InUsage)
    { 
        NumBuffers = GetNumBuffersFromUsage(InUsage);
    }
    uint8 DynamicBufferIndex;
    uint8 NumBuffers;
    VkBuffer Buffers[NUM_BUFFERS];
    VkDeviceMemory Memories[NUM_BUFFERS];
    void* Lock(class FVulkanDynamicRHI* Context, EDataAccessFlag Access, uint32 LockSize, uint32 Offset);
    void Unlock();
    static uint8 GetNumBuffersFromUsage(EBufferUsageFlags InUsage)
    {
		const bool bDynamic = EnumHasAnyFlags(InUsage, EBufferUsageFlags::Dynamic);
		return bDynamic ? NUM_BUFFERS : 1;
    }
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