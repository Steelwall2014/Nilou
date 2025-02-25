#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "Common/EnumClassFlags.h"

namespace nilou {

class FVulkanCmdBuffer;

struct FStagingBuffer
{
    VkDevice Device{};
    VkBuffer Buffer{};
    VkDeviceMemory StagingBufferMemory{};
    VkMemoryPropertyFlagBits MemoryReadFlags{};
    uint32 BufferSize{};
    void* MappedPointer{};

    ~FStagingBuffer();

    void* GetMappedPointer();

    bool IsCoherent() const
    {
        return MemoryReadFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT != 0;
    }

    uint32 GetSize() const { return BufferSize; }

    void FlushMappedMemory();
    void InvalidateMappedMemory();
};
// using FStagingBufferRef = std::shared_ptr<FStagingBuffer>;

struct FPendingBufferLock
{
    FStagingBuffer* StagingBuffer;
    uint32 Offset;
    uint32 Size;
    EResourceLockMode LockMode;
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

    void Deinit();

    FStagingBuffer* AcquireBuffer(uint32 Size, VkBufferUsageFlags InUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VkMemoryPropertyFlagBits InMemoryReadFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Sets pointer to nullptr
    void ReleaseBuffer(FVulkanCmdBuffer* CmdBuffer, FStagingBuffer*& StagingBuffer);

    void ProcessPendingFree(bool bImmediately, bool bFreeToOS);

protected:
    friend class FMemoryManager;
    struct FPendingItemsPerCmdBuffer
    {
        FVulkanCmdBuffer* CmdBuffer;
        struct FPendingItems
        {
            uint64 FenceCounter;
            std::vector<FStagingBuffer*> Resources;
        };


        inline FPendingItems* FindOrAddItemsForFence(uint64 Fence);

        std::vector<FPendingItems> PendingItems;
    };

    std::mutex StagingLock;

    std::vector<FStagingBuffer*> UsedStagingBuffers;
    std::vector<FPendingItemsPerCmdBuffer> PendingFreeStagingBuffers;

    struct FFreeEntry
    {
        FStagingBuffer* StagingBuffer;
        uint32 FrameNumber;
    };
    std::vector<FFreeEntry> FreeStagingBuffers;

    uint64 PeakUsedMemory;
    uint64 UsedMemory;

    FPendingItemsPerCmdBuffer* FindOrAdd(FVulkanCmdBuffer* CmdBuffer);
    void ProcessPendingFreeNoLock(bool bImmediately, bool bFreeToOS);

    VkDevice Device;
    FVulkanDynamicRHI* VulkanRHI;
};

class VulkanMultiBuffer
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
    VulkanMultiBuffer(FVulkanDynamicRHI* Context, uint32 InSize, EBufferUsageFlags InUsage, VkBufferUsageFlags InVkUsage);
    uint8 DynamicBufferIndex = 0;
    uint8 NumBuffers;
    VkBuffer Buffers[NUM_BUFFERS];
    VkDeviceMemory Memories[NUM_BUFFERS];
    EBufferUsageFlags Usage;
    uint32 Size;
    FVulkanDynamicRHI* Context;
    void* MappedPointers[NUM_BUFFERS]{};
    void* Lock(class FVulkanDynamicRHI* Context, EResourceLockMode LockMode, uint32 LockSize, uint32 Offset);
    void Unlock(FVulkanDynamicRHI* Context);
    static uint8 GetNumBuffersFromUsage(EBufferUsageFlags InUsage)
    {
		const bool bDynamic = EnumHasAnyFlags(InUsage, EBufferUsageFlags::Dynamic);
		return bDynamic ? NUM_BUFFERS : 1;
    }
    ~VulkanMultiBuffer();
};

class VulkanBuffer : public RHIBuffer
{
public:
    VulkanBuffer(FVulkanDynamicRHI* Context, uint32 InStride, uint32 InSize, EBufferUsageFlags InUsage)
        : RHIBuffer(InStride, InSize, InUsage)
        // , MultiBuffer(Context, InSize, InUsage, 0)
    { 
    }
    /** DEPRECATED */
    // VulkanMultiBuffer MultiBuffer;
    // VkBuffer GetHandle() const { return MultiBuffer.Buffers[MultiBuffer.DynamicBufferIndex]; }
    // void* Lock(class FVulkanDynamicRHI* Context, EResourceLockMode LockMode, uint32 LockSize, uint32 Offset) { return MultiBuffer.Lock(Context, LockMode, LockSize, Offset); }
    // void Unlock(FVulkanDynamicRHI* Context) { MultiBuffer.Unlock(Context); }
    /** DEPRECATED */

    VkBuffer Handle{};
    VkDeviceMemory Memory{};
};
using VulkanBufferRef = TRefCountPtr<VulkanBuffer>;

class VulkanUniformBuffer : public RHIUniformBuffer
{
public:
    VulkanUniformBuffer(FVulkanDynamicRHI* Context, uint32 InSize, EUniformBufferUsage InUsage)
        : RHIUniformBuffer(InSize, InUsage)
        , MultiBuffer(Context, InSize, EBufferUsageFlags::Dynamic, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    { }
    VulkanMultiBuffer MultiBuffer;
    VkBuffer GetHandle() const { return MultiBuffer.Buffers[MultiBuffer.DynamicBufferIndex]; }
    void* Lock(class FVulkanDynamicRHI* Context, EResourceLockMode LockMode, uint32 LockSize, uint32 Offset) { return MultiBuffer.Lock(Context, LockMode, LockSize, Offset); }
    void Unlock(FVulkanDynamicRHI* Context) { MultiBuffer.Unlock(Context); }
};
using VulkanUniformBufferRef = TRefCountPtr<VulkanUniformBuffer>;

inline VulkanBuffer* ResourceCast(RHIBuffer* Buffer)
{
    return static_cast<VulkanBuffer*>(Buffer);
}

inline VulkanUniformBuffer* ResourceCast(RHIUniformBuffer* Buffer)
{
    return static_cast<VulkanUniformBuffer*>(Buffer);
}

}