#include "VulkanBuffer.h"
#include "VulkanDynamicRHI.h"
#include "Common/EnumClassFlags.h"
#include "VulkanCommandBuffer.h"
#include "VulkanMemory.h"
#include "RenderingThread.h"

namespace nilou {


static VkBufferUsageFlags TranslateBufferUsageFlags(EBufferUsageFlags InUsage, bool bZeroSize)
{
	// Always include TRANSFER_SRC since hardware vendors confirmed it wouldn't have any performance cost and we need it for some debug functionalities.
	VkBufferUsageFlags OutVkUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	auto TranslateFlag = [&OutVkUsage, &InUsage](EBufferUsageFlags SearchFlag, VkBufferUsageFlags AddedIfFound, VkBufferUsageFlags AddedIfNotFound = 0)
	{
		const bool HasFlag = EnumHasAnyFlags(InUsage, SearchFlag);
		OutVkUsage |= HasFlag ? AddedIfFound : AddedIfNotFound;
	};

	TranslateFlag(EBufferUsageFlags::VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	TranslateFlag(EBufferUsageFlags::IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	TranslateFlag(EBufferUsageFlags::StructuredBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	if (!bZeroSize)
	{
		TranslateFlag(EBufferUsageFlags::UnorderedAccess, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
		TranslateFlag(EBufferUsageFlags::DrawIndirect, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
		TranslateFlag(EBufferUsageFlags::KeepCPUAccessible, (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT));
		TranslateFlag(EBufferUsageFlags::ShaderResource, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);

		TranslateFlag(EBufferUsageFlags::Volatile, 0, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	}

	return OutVkUsage;
}


static std::map<VulkanMultiBuffer*, FPendingBufferLock> GPendingLockIBs;
static std::mutex GPendingLockIBsMutex;

static FPendingBufferLock GetPendingBufferLock(VulkanMultiBuffer* Buffer)
{
	FPendingBufferLock PendingLock;

	// Found only if it was created for Write
	std::lock_guard<std::mutex> ScopeLock(GPendingLockIBsMutex);
    if (GPendingLockIBs.contains(Buffer))
    {
        PendingLock = GPendingLockIBs[Buffer];
        GPendingLockIBs.erase(Buffer);
    }
	return PendingLock;
}

FStagingBuffer::~FStagingBuffer()
{
    FVulkanDynamicRHI* VulkanRHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI());
    VulkanRHI->MemoryManager->FreeMemory(StagingBufferMemory);
    vkUnmapMemory(Device, StagingBufferMemory);
}

void* FStagingBuffer::GetMappedPointer()
{
    if (!MappedPointer)
    {
        vkMapMemory(Device, StagingBufferMemory, 0, BufferSize, 0, &MappedPointer);
    }
    return MappedPointer;
}

void FStagingBuffer::FlushMappedMemory()
{
    if (!IsCoherent())
    {
        VkMappedMemoryRange Range{};
        Range.memory = StagingBufferMemory;
        Range.offset = 0;
        Range.size = BufferSize;
        vkFlushMappedMemoryRanges(Device, 1, &Range);
    }
}

void FStagingBuffer::InvalidateMappedMemory()
{
    if (!IsCoherent())
    {
        VkMappedMemoryRange Range{};
        Range.memory = StagingBufferMemory;
        Range.offset = 0;
        Range.size = BufferSize;
        vkInvalidateMappedMemoryRanges(Device, 1, &Range);
    }
}

template <typename T>
constexpr T AlignArbitrary(T Val, uint64 Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignArbitrary expects an integer or pointer type");

	return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
}

FStagingBuffer* FVulkanStagingManager::AcquireBuffer(uint32 Size, VkBufferUsageFlags InUsageFlags, VkMemoryPropertyFlagBits InMemoryReadFlags)
{
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
            FFreeEntry& FreeBuffer = FreeStagingBuffers[Index];
            if (FreeBuffer.StagingBuffer->BufferSize == Size && FreeBuffer.StagingBuffer->MemoryReadFlags == InMemoryReadFlags)
            {
                FStagingBufferRef Buffer = FreeBuffer.StagingBuffer;
                FreeStagingBuffers.erase(FreeStagingBuffers.begin()+Index);
                UsedStagingBuffers.push_back(Buffer);
                return Buffer.get();
            }
        }
    }

    FStagingBufferRef StagingBuffer = std::make_shared<FStagingBuffer>();
    StagingBuffer->MemoryReadFlags = InMemoryReadFlags;
    StagingBuffer->BufferSize = Size;
    StagingBuffer->Device = Device;
    

    VkBufferCreateInfo StagingBufferCreateInfo;
    StagingBufferCreateInfo.flags = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    StagingBufferCreateInfo.size = Size;
    StagingBufferCreateInfo.usage = InUsageFlags;

    vkCreateBuffer(Device, &StagingBufferCreateInfo, nullptr, &StagingBuffer->Buffer);

    VulkanRHI->MemoryManager->AllocateBufferMemory(&StagingBuffer->StagingBufferMemory, StagingBuffer->Buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | InMemoryReadFlags);

    {
        std::lock_guard<std::mutex> Lock(StagingLock);
        UsedStagingBuffers.push_back(StagingBuffer);
        UsedMemory += StagingBuffer->BufferSize;
        PeakUsedMemory = std::max(UsedMemory, PeakUsedMemory);
    }

    return StagingBuffer.get();
}

void FVulkanStagingManager::ReleaseBuffer(FVulkanCmdBuffer* CmdBuffer, FStagingBuffer*& StagingBuffer)
{
    std::lock_guard<std::mutex> Lock(StagingLock);
    FStagingBufferRef StagingBufferRef = nullptr;
    for (auto iter = UsedStagingBuffers.begin(); iter != UsedStagingBuffers.end(); iter++)
    {
        if (iter->get() == StagingBuffer)
        {
            StagingBufferRef = *iter;
            UsedStagingBuffers.erase(iter);
            break;
        }
    }

    // if (CmdBuffer)
    // {
    //     FPendingItemsPerCmdBuffer* ItemsForCmdBuffer = FindOrAdd(CmdBuffer);
    //     FPendingItemsPerCmdBuffer::FPendingItems* ItemsForFence = ItemsForCmdBuffer->FindOrAddItemsForFence(CmdBuffer->GetFenceSignaledCounterA());
    //     check(StagingBuffer);
    //     ItemsForFence->Resources.Add(StagingBuffer);
    // }
    // else
    // {
        FreeStagingBuffers.push_back({StagingBufferRef, FRenderingThread::GetFrameCount()});
    // }
    StagingBuffer = nullptr;
}

VulkanMultiBuffer::VulkanMultiBuffer(FVulkanDynamicRHI* Context, uint32 InSize, EBufferUsageFlags InUsage)
    : Size(InSize)
    , Usage(InUsage)
{ 
    NumBuffers = GetNumBuffersFromUsage(InUsage);

    const bool bZeroSize = (Size == 0);
    VkBufferUsageFlags UsageFlags = TranslateBufferUsageFlags(InUsage, bZeroSize);
    
    auto properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkDevice Device = Context->device;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = Size;
    bufferInfo.usage = UsageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    for (int i = 0; i < NumBuffers; i++)
    {
        if (vkCreateBuffer(Device, &bufferInfo, nullptr, &Buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        Context->MemoryManager->AllocateBufferMemory(&Memories[i], Buffers[i], properties);

        vkBindBufferMemory(Device, Buffers[i], Memories[i], 0);
    }
}

void* VulkanMultiBuffer::Lock(FVulkanDynamicRHI* Context, EResourceLockMode LockMode, uint32 LockSize, uint32 Offset)
{
	void* Data = nullptr;
	uint32 DataOffset = 0;

	const bool bDynamic = EnumHasAnyFlags(Usage, EBufferUsageFlags::Dynamic);
	const bool bVolatile = EnumHasAnyFlags(Usage, EBufferUsageFlags::Volatile);
	const bool bStatic = EnumHasAnyFlags(Usage, EBufferUsageFlags::Static) || !(bVolatile || bDynamic);
	const bool bUAV = EnumHasAnyFlags(Usage, EBufferUsageFlags::UnorderedAccess);
	const bool bSR = EnumHasAnyFlags(Usage, EBufferUsageFlags::ShaderResource);

    if (LockMode == RLM_ReadOnly)
    {
        FVulkanCmdBuffer* CmdBuffer = Context->GetCommandBufferManager()->GetUploadCmdBuffer();
        VkMemoryBarrier BarrierBefore = { VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT };
        vkCmdPipelineBarrier(CmdBuffer->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &BarrierBefore, 0, nullptr, 0, nullptr);
    
        FStagingBuffer* StagingBuffer = Context->StagingManager->AcquireBuffer(LockSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
        
        VkBufferCopy Regions;
        Regions.size = LockSize;
        Regions.srcOffset = Offset;
        Regions.dstOffset = 0;

        vkCmdCopyBuffer(CmdBuffer->GetHandle(), Buffers[DynamicBufferIndex], StagingBuffer->Buffer, 1, &Regions);

        // Setup barrier.
        VkMemoryBarrier BarrierAfter = { VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_HOST_READ_BIT };
        vkCmdPipelineBarrier(CmdBuffer->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &BarrierAfter, 0, nullptr, 0, nullptr);
        
        // Force upload.
        Context->CommandBufferManager->SubmitUploadCmdBuffer();
        //Device->WaitUntilIdle();

        // Flush.
        StagingBuffer->FlushMappedMemory();

        // Get mapped pointer. 
        Data = StagingBuffer->GetMappedPointer();

        // Release temp staging buffer during unlock.
        FPendingBufferLock PendingLock;
        PendingLock.Offset = 0;
        PendingLock.Size = LockSize;
        PendingLock.LockMode = LockMode;
        PendingLock.StagingBuffer = StagingBuffer;

        {
            std::lock_guard<std::mutex> ScopeLock(GPendingLockIBsMutex);
            GPendingLockIBs[this] = PendingLock;
        }

        Context->CommandBufferManager->PrepareForNewActiveCommandBuffer();
    }
    else 
    {
        check(LockMode == RLM_WriteOnly);
    
        DynamicBufferIndex = (DynamicBufferIndex + 1) % NumBuffers;
        if (bStatic)
        {
            FPendingBufferLock PendingLock;
            PendingLock.Offset = Offset;
            PendingLock.Size = LockSize;
            PendingLock.LockMode = LockMode;

            FStagingBuffer* StagingBuffer = Context->StagingManager->AcquireBuffer(LockSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            PendingLock.StagingBuffer = StagingBuffer;
            Data = StagingBuffer->GetMappedPointer();

            {
                std::lock_guard<std::mutex> ScopeLock(GPendingLockIBsMutex);
                check(!GPendingLockIBs.contains(this));
                GPendingLockIBs[this] = PendingLock;
            }
        }
        else
        {
            FVulkanDynamicRHI* VulkanRHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI());
            vkMapMemory(VulkanRHI->device, Memories[DynamicBufferIndex], 0, Size, 0, &Data);
            DataOffset = Offset;
            LockStatus = ELockStatus::PersistentMapping;
        }
    }

}

void VulkanMultiBuffer::Unlock(FVulkanDynamicRHI* Context)
{
	const bool bDynamic = EnumHasAnyFlags(Usage, EBufferUsageFlags::Dynamic);
	const bool bVolatile = EnumHasAnyFlags(Usage, EBufferUsageFlags::Volatile);
	const bool bStatic = EnumHasAnyFlags(Usage, EBufferUsageFlags::Static) || !(bVolatile || bDynamic);
	const bool bSR = EnumHasAnyFlags(Usage, EBufferUsageFlags::ShaderResource);

	check(LockStatus != ELockStatus::Unlocked);

	if (bVolatile || LockStatus == ELockStatus::PersistentMapping)
	{
		// Nothing to do here...
	}
	else
	{
		check(bStatic || bDynamic || bSR);

		FPendingBufferLock PendingLock = GetPendingBufferLock(this);

		PendingLock.StagingBuffer->FlushMappedMemory();

		if (PendingLock.LockMode == RLM_ReadOnly)
		{
			// Just remove the staging buffer here.
			Context->StagingManager->ReleaseBuffer(0, PendingLock.StagingBuffer);
		}
		else if (PendingLock.LockMode == RLM_WriteOnly)
		{
            FStagingBuffer* StagingBuffer = PendingLock.StagingBuffer;
            // We need to do this on the active command buffer instead of using an upload command buffer. The high level code sometimes reuses the same
            // buffer in sequences of upload / dispatch, upload / dispatch, so we need to order the copy commands correctly with respect to the dispatches.
            FVulkanCmdBuffer* Cmd = Context->CommandBufferManager->GetActiveCmdBuffer();
            check(Cmd && Cmd->IsOutsideRenderPass());
            VkCommandBuffer CmdBuffer = Cmd->GetHandle();
	        VkBufferCopy Region{};
            Region.size = PendingLock.Size;
            Region.dstOffset = PendingLock.Offset;
            vkCmdCopyBuffer(CmdBuffer, StagingBuffer->Buffer, Buffers[DynamicBufferIndex], 1, &Region);
            VkMemoryBarrier BarrierAfter = { VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT };
	        vkCmdPipelineBarrier(CmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &BarrierAfter, 0, nullptr, 0, nullptr);
            Context->StagingManager->ReleaseBuffer(Cmd, StagingBuffer);
		}
	}

	LockStatus = ELockStatus::Unlocked;
}

RHIBufferRef FVulkanDynamicRHI::RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data)
{
    VulkanBufferRef Buffer = std::make_shared<VulkanBuffer>(this, Stride, Size, InUsage);

    return Buffer;
}

RHIUniformBufferRef FVulkanDynamicRHI::RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data)
{
    VulkanUniformBufferRef Buffer = std::make_shared<VulkanUniformBuffer>(this, Size, InUsage);

    return Buffer;

}

RHIBufferRef FVulkanDynamicRHI::RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data)
{
    return RHICreateBuffer(DataByteLength, DataByteLength, EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::Dynamic, Data);
}

RHIBufferRef FVulkanDynamicRHI::RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z)
{
    DispatchIndirectCommand command{ num_groups_x, num_groups_y, num_groups_z };
    return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::DispatchIndirect | EBufferUsageFlags::Dynamic, &command);
}

RHIBufferRef FVulkanDynamicRHI::RHICreateDrawElementsIndirectBuffer(
    int32 Count, uint32 instanceCount, uint32 firstIndex, uint32 baseVertex, uint32 baseInstance)
{
    DrawElementsIndirectCommand command{ Count, instanceCount, firstIndex, baseVertex, baseInstance };
    return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::DrawIndirect | EBufferUsageFlags::Dynamic, &command);
}

void *FVulkanDynamicRHI::RHILockBuffer(RHIBufferRef buffer, EResourceLockMode LockMode)
{
    VulkanBuffer* VulknaBuffer = static_cast<VulkanBuffer*>(buffer.get());
    return VulknaBuffer->Lock(this, LockMode, VulknaBuffer->GetSize(), 0);
}

void FVulkanDynamicRHI::RHIUnlockBuffer(RHIBufferRef buffer)
{
    VulkanBuffer* VulknaBuffer = static_cast<VulkanBuffer*>(buffer.get());
    VulknaBuffer->Unlock(this);
}


}