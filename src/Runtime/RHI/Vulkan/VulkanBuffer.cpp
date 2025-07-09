#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanDynamicRHI.h"
#include "Common/EnumClassFlags.h"
#include "VulkanCommandBuffer.h"
#include "VulkanMemory.h"
#include "RenderingThread.h"

namespace nilou {


static VkBufferUsageFlags TranslateBufferUsageFlags(EBufferUsageFlags InUsage)
{
	return (VkBufferUsageFlags)InUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
}
static VkMemoryPropertyFlags TranslateMemoryPropertyFlags(EBufferUsageFlags InUsage)
{
	VkMemoryPropertyFlags Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::StorageBuffer) ||
		EnumHasAnyFlags(InUsage, EBufferUsageFlags::TransferSrc))
	{
		Properties |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	return Properties;
}

template <typename T>
constexpr T AlignArbitrary(T Val, uint64 Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignArbitrary expects an integer or pointer type");

	return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
}

RHIBuffer* FVulkanStagingManager::AcquireBuffer(uint32 Size, VkBufferUsageFlags InUsageFlags, VkMemoryPropertyFlags InMemoryReadFlags)
{
	FVulkanDynamicRHI* VulkanRHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::Get());
    if (InMemoryReadFlags == VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
    {
        uint64 NonCoherentAtomSize = (uint64)VulkanRHI->GpuProps.limits.nonCoherentAtomSize;
        Size = AlignArbitrary(Size, NonCoherentAtomSize);
    }

    // Add both source and dest flags
    if ((InUsageFlags & (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)) != 0)
    {
        InUsageFlags |= (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    }

    //#todo-rco: Better locking!
    {
        std::lock_guard<std::mutex> Lock(StagingLock);
        for (int32 Index = FreeStagingBuffers.size()-1; Index >= 0; --Index)
        {
            RHIBufferRef FreeBuffer = FreeStagingBuffers[Index];
            if (FreeBuffer->GetSize() == Size && ResourceCast(FreeBuffer.GetReference())->MemoryReadFlags == InMemoryReadFlags)
            {
                FreeStagingBuffers.erase(FreeStagingBuffers.begin()+Index);
                UsedStagingBuffers.push_back(FreeBuffer);
                return FreeBuffer.GetReference();
            }
        }
    }

    RHIBufferRef StagingBuffer = VulkanRHI->RHICreateBufferInternal(Size, Size, EBufferUsageFlags::TransferSrc | EBufferUsageFlags::TransferDst, InUsageFlags, InMemoryReadFlags);

    {
        std::lock_guard<std::mutex> Lock(StagingLock);
        UsedStagingBuffers.push_back(StagingBuffer);
        UsedMemory += StagingBuffer->GetSize();
        PeakUsedMemory = std::max(UsedMemory, PeakUsedMemory);
    }

    return StagingBuffer.GetReference();
}

void FVulkanStagingManager::ReleaseBuffer(RHIBuffer*& StagingBuffer)
{
    std::lock_guard<std::mutex> Lock(StagingLock);
	FreeStagingBuffers.push_back(TRefCountPtr(StagingBuffer));
    UsedStagingBuffers.erase(std::find(UsedStagingBuffers.begin(), UsedStagingBuffers.end(), StagingBuffer));
    StagingBuffer = nullptr;
}

// FVulkanStagingManager::FPendingItemsPerCmdBuffer::FPendingItems* FVulkanStagingManager::FPendingItemsPerCmdBuffer::FindOrAddItemsForFence(uint64 Fence)
// {
//     for (int32 Index = 0; Index < PendingItems.size(); ++Index)
//     {
//         if (PendingItems[Index].FenceCounter == Fence)
//         {
//             return &PendingItems[Index];
//         }
//     }

//     FPendingItems& New = PendingItems.emplace_back();
//     New.FenceCounter = Fence;
//     return &New;
// }

// FVulkanStagingManager::FPendingItemsPerCmdBuffer* FVulkanStagingManager::FindOrAdd(FVulkanCmdBuffer* CmdBuffer)
// {
//     for (int32 Index = 0; Index < PendingFreeStagingBuffers.size(); ++Index)
//     {
//         if (PendingFreeStagingBuffers[Index].CmdBuffer == CmdBuffer)
//         {
//             return &PendingFreeStagingBuffers[Index];
//         }
//     }

//     FPendingItemsPerCmdBuffer& New = PendingFreeStagingBuffers.emplace_back();
//     New.CmdBuffer = CmdBuffer;
//     return &New;
// }

// void FVulkanStagingManager::ProcessPendingFree(bool bImmediately, bool bFreeToOS)
// {
//     std::lock_guard<std::mutex> Lock(StagingLock);
//     ProcessPendingFreeNoLock(bImmediately, bFreeToOS);
// }

// void FVulkanStagingManager::ProcessPendingFreeNoLock(bool bImmediately, bool bFreeToOS)
// {
//     int32 NumOriginalFreeBuffers = FreeStagingBuffers.size();
//     for (int32 Index = PendingFreeStagingBuffers.size() - 1; Index >= 0; --Index)
//     {
//         FPendingItemsPerCmdBuffer& EntriesPerCmdBuffer = PendingFreeStagingBuffers[Index];
//         for (int32 FenceIndex = EntriesPerCmdBuffer.PendingItems.size() - 1; FenceIndex >= 0; --FenceIndex)
//         {
//             FPendingItemsPerCmdBuffer::FPendingItems& PendingItems = EntriesPerCmdBuffer.PendingItems[FenceIndex];
//             if (bImmediately || PendingItems.FenceCounter < EntriesPerCmdBuffer.CmdBuffer->GetFenceSignaledCounter())
//             {
//                 for (int32 ResourceIndex = 0; ResourceIndex < PendingItems.Resources.size(); ++ResourceIndex)
//                 {
//                     Ncheck(PendingItems.Resources[ResourceIndex]);
//                     FreeStagingBuffers.push_back({PendingItems.Resources[ResourceIndex], FRenderingThread::GetFrameCount()});
//                 }

//                 EntriesPerCmdBuffer.PendingItems.erase(EntriesPerCmdBuffer.PendingItems.begin() + FenceIndex);
//             }
//         }

//         if (EntriesPerCmdBuffer.PendingItems.size() == 0)
//         {
//             PendingFreeStagingBuffers.erase(PendingFreeStagingBuffers.begin() + Index);
//         }
//     }

//     if (bFreeToOS)
//     {
//         int32 NumFreeBuffers = bImmediately ? FreeStagingBuffers.size() : NumOriginalFreeBuffers;
//         for (int32 Index = NumFreeBuffers - 1; Index >= 0; --Index)
//         {
//             FFreeEntry& Entry = FreeStagingBuffers[Index];
//             if (bImmediately || Entry.FrameNumber < FRenderingThread::GetFrameCount())
//             {
//                 UsedMemory -= Entry.StagingBuffer->GetSize();
//                 delete Entry.StagingBuffer;
//                 FreeStagingBuffers.erase(FreeStagingBuffers.begin()+Index);
//             }
//         }
//     }
// }

VulkanBuffer::~VulkanBuffer()
{
	if (Handle)
	{
		vkDestroyBuffer(Device->Handle, Handle, nullptr);
		Handle = VK_NULL_HANDLE;
	}
	if (Memory)
	{
		vkFreeMemory(Device->Handle, Memory, nullptr);
		Memory = VK_NULL_HANDLE;
	}
}

RHIBufferRef FVulkanDynamicRHI::RHICreateBufferInternal(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, VkBufferUsageFlags UsageFlags, VkMemoryPropertyFlags MemoryReadFlags)
{
    VulkanBufferRef Buffer = TRefCountPtr(new VulkanBuffer(Device, Stride, Size, InUsage));

	VkBufferCreateInfo BufferInfo{};
	BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferInfo.size = Size;
	BufferInfo.usage = UsageFlags;
	BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VK_CHECK_RESULT(vkCreateBuffer(Device->Handle, &BufferInfo, nullptr, &Buffer->Handle));

    Device->MemoryManager->AllocateBufferMemory(&Buffer->Memory, Buffer->Handle, MemoryReadFlags);
    VK_CHECK_RESULT(vkBindBufferMemory(Device->Handle, Buffer->Handle, Buffer->Memory, 0));

	Buffer->UsageFlags = UsageFlags;
	Buffer->MemoryReadFlags = MemoryReadFlags;

    return Buffer;
}

RHIBufferRef FVulkanDynamicRHI::RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, const void *Data)
{
	return RHICreateBufferInternal(Stride, Size, InUsage, TranslateBufferUsageFlags(InUsage), TranslateMemoryPropertyFlags(InUsage));
}

RHIBufferRef FVulkanDynamicRHI::RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data)
{
    return RHICreateBuffer(DataByteLength, DataByteLength, EBufferUsageFlags::StorageBuffer, Data);
}

RHIBufferRef FVulkanDynamicRHI::RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z)
{
    VkDispatchIndirectCommand command{ num_groups_x, num_groups_y, num_groups_z };
    return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::IndirectBuffer, &command);
}

RHIBufferRef FVulkanDynamicRHI::RHICreateDrawElementsIndirectBuffer(
    int32 Count, uint32 instanceCount, uint32 firstIndex, uint32 baseVertex, uint32 baseInstance)
{
    VkDrawIndexedIndirectCommand command{ (uint32)Count, instanceCount, firstIndex, (int32)baseVertex, baseInstance };
    return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::IndirectBuffer, &command);
}

// void *FVulkanDynamicRHI::RHILockBuffer(RHIBuffer* buffer, uint32 Offset, uint32 Size, EResourceLockMode LockMode)
// {
//     VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
//     return vkBuffer->Lock(this, LockMode, Size, Offset);
// }

// void FVulkanDynamicRHI::RHIUnlockBuffer(RHIBuffer* buffer)
// {
//     VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
//     vkBuffer->Unlock(this);
// }

// void FVulkanDynamicRHI::RHIBindBufferData(RHIBuffer* buffer, unsigned int size, void *data)
// {
//     void* Dst = RHILockBuffer(buffer, 0, size, RLM_WriteOnly);
//         std::memcpy(Dst, data, size);
//     RHIUnlockBuffer(buffer);
// }

// void FVulkanDynamicRHI::RHIUpdateBuffer(RHIBuffer* Buffer, uint32 Offset, uint32 Size, void* Data)
// {
//     void* Dst = RHILockBuffer(Buffer, Offset, Size, RLM_WriteOnly);
//         std::memcpy(Dst, Data, Size);
//     RHIUnlockBuffer(Buffer);
// }

// void FVulkanDynamicRHI::RHIUpdateUniformBuffer(RHIUniformBufferRef Buffer, void* Data)
// {
//     VulkanUniformBuffer* vkBuffer = static_cast<VulkanUniformBuffer*>(Buffer.get());
//     int32 Size = Buffer->GetSize();
//     void* Dst = vkBuffer->Lock(this, RLM_WriteOnly, Size, 0);
//         std::memcpy(Dst, Data, Size);
//     vkBuffer->Unlock(this);
// }

// void FVulkanDynamicRHI::RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size)
// {
//     FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetUploadCmdBuffer();
//     VulkanBuffer* vkReadBuffer = static_cast<VulkanBuffer*>(readBuffer.get());
//     VulkanBuffer* vkWriteBuffer = static_cast<VulkanBuffer*>(writeBuffer.get());
//     VkBufferCopy Region{};
//     Region.size = size;
//     Region.srcOffset = readOffset;
//     Region.dstOffset = writeOffset;
//     vkCmdCopyBuffer(CmdBuffer->GetHandle(), vkReadBuffer->GetHandle(), vkWriteBuffer->GetHandle(), 1, &Region);
//     VkMemoryBarrier BarrierAfter = { VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT };
//     vkCmdPipelineBarrier(CmdBuffer->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &BarrierAfter, 0, nullptr, 0, nullptr);
//     CommandBufferManager->SubmitUploadCmdBuffer();
// }

void* FVulkanDynamicRHI::RHIMapMemory(RHIBuffer* buffer, uint32 Offset, uint32 Size)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    Ncheck(vkBuffer);
    Ncheck(vkBuffer->Memory);
    void* MappedPointer = nullptr;
    vkMapMemory(Device->Handle, vkBuffer->Memory, Offset, Size, 0, &MappedPointer);
    return MappedPointer;
}

void FVulkanDynamicRHI::RHIUnmapMemory(RHIBuffer* buffer)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    Ncheck(vkBuffer);
    Ncheck(vkBuffer->Memory);
    vkUnmapMemory(Device->Handle, vkBuffer->Memory);
}

}