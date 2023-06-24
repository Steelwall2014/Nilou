#include "VulkanBuffer.h"
#include "VulkanDynamicRHI.h"
#include "Common/EnumClassFlags.h"
#include "VulkanCommandBuffer.h"
#include "VulkanMemory.h"
#include "RenderingThread.h"

namespace nilou {

static std::map<VulkanBuffer*, FPendingBufferLock> GPendingLockIBs;
static std::mutex GPendingLockIBsMutex;

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

    FreeStagingBuffers.push_back({StagingBufferRef, FRenderingThread::GetFrameCount()});
    StagingBuffer = nullptr;
}

void* VulkanBuffer::Lock(FVulkanDynamicRHI* Context, EDataAccessFlag Access, uint32 LockSize, uint32 Offset)
{
	void* Data = nullptr;
	uint32 DataOffset = 0;

	const bool bDynamic = EnumHasAnyFlags(GetUsage(), Dynamic);
	const bool bVolatile = EnumHasAnyFlags(GetUsage(), Volatile);
	const bool bStatic = EnumHasAnyFlags(GetUsage(), Static) || !(bVolatile || bDynamic);
	const bool bUAV = EnumHasAnyFlags(GetUsage(), UnorderedAccess);
	const bool bSR = EnumHasAnyFlags(GetUsage(), ShaderResource);

    if (Access == EDataAccessFlag::DA_ReadOnly)
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
        PendingLock.LockMode = Access;
        PendingLock.StagingBuffer = StagingBuffer;

        {
            std::lock_guard<std::mutex> ScopeLock(GPendingLockIBsMutex);
            GPendingLockIBs[this] = PendingLock;
        }

        Context->CommandBufferManager->PrepareForNewActiveCommandBuffer();
    }
    else 
    {
        check(Access == EDataAccessFlag::DA_WriteOnly || Access == EDataAccessFlag::DA_ReadWrite);
    
        DynamicBufferIndex = (DynamicBufferIndex + 1) % NumBuffers;
        if (bStatic)
        {
            FPendingBufferLock PendingLock;
            PendingLock.Offset = Offset;
            PendingLock.Size = LockSize;
            PendingLock.LockMode = Access;

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

void VulkanBuffer::Unlock()
{
	check(RHICmdList || Context);

	const bool bDynamic = EnumHasAnyFlags(GetUsage(), Dynamic);
	const bool bVolatile = EnumHasAnyFlags(GetUsage(), Volatile);
	const bool bStatic = EnumHasAnyFlags(GetUsage(), Static) || !(bVolatile || bDynamic);
	const bool bSR = EnumHasAnyFlags(GetUsage(), ShaderResource);

	check(LockStatus != ELockStatus::Unlocked);

	if (bVolatile || LockStatus == ELockStatus::PersistentMapping)
	{
		// Nothing to do here...
	}
	else
	{
		check(bStatic || bDynamic || bSR);

		VulkanRHI::FPendingBufferLock PendingLock = GetPendingBufferLock(this);

		PendingLock.StagingBuffer->FlushMappedMemory();

		if (PendingLock.LockMode == RLM_ReadOnly)
		{
			// Just remove the staging buffer here.
			Device->GetStagingManager().ReleaseBuffer(0, PendingLock.StagingBuffer);
		}
		else if (PendingLock.LockMode == RLM_WriteOnly)
		{
			if (Context || (RHICmdList && RHICmdList->IsBottomOfPipe()))
			{
				if (!Context)
				{
					Context = &FVulkanCommandListContext::GetVulkanContext(RHICmdList->GetContext());
				}

				FVulkanResourceMultiBuffer::InternalUnlock(*Context, PendingLock, this, DynamicBufferIndex);
			}
			else
			{
				ALLOC_COMMAND_CL(*RHICmdList, FRHICommandMultiBufferUnlock)(Device, PendingLock, this, DynamicBufferIndex);
			}
		}
	}

	LockStatus = ELockStatus::Unlocked;
}

VulkanBuffer::~VulkanBuffer()
{
    FVulkanDynamicRHI* RHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI());
    vkDestroyBuffer(RHI->device, Buffer, nullptr);
    vkFreeMemory(RHI->device, Memory, nullptr);
    RHIBuffer::~RHIBuffer();
}

VulkanUniformBuffer::~VulkanUniformBuffer()
{
    FVulkanDynamicRHI* RHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI());
    vkDestroyBuffer(RHI->device, Buffer, nullptr);
    vkFreeMemory(RHI->device, Memory, nullptr);
    RHIUniformBuffer::~RHIUniformBuffer();
}

VkBufferUsageFlags TranslateBufferUsageFlags(EBufferUsageFlags InUsage, bool bZeroSize)
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

void FVulkanDynamicRHI::RHICreateBufferInternal(
                    VkDevice Device, VkBufferUsageFlags UsageFlags,
                    uint32 Size, void *Data, 
                    VkBuffer* Buffer, VkDeviceMemory* Memory)
{
    auto properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = Size;
    bufferInfo.usage = UsageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(Device, &bufferInfo, nullptr, Buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    MemoryManager->AllocateBufferMemory(Memory, *Buffer, properties);

    vkBindBufferMemory(Device, *Buffer, *Memory, 0);
}

RHIBufferRef FVulkanDynamicRHI::RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data)
{
    VulkanBufferRef Buffer = std::make_shared<VulkanBuffer>(Stride, Size, InUsage);

    const bool bZeroSize = (Size == 0);
    RHICreateBufferInternal(device, TranslateBufferUsageFlags(InUsage, bZeroSize), Size, Data, &Buffer->Buffers[0], &Buffer->Memories[0]);

    return Buffer;
}

RHIUniformBufferRef FVulkanDynamicRHI::RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data)
{
    VulkanUniformBufferRef Buffer = std::make_shared<VulkanUniformBuffer>(Size, InUsage);

    CreateBuffer(device, TranslateBufferUsageFlags(InUsage, bZ))
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

void *FVulkanDynamicRHI::RHILockBuffer(RHIBufferRef buffer, EDataAccessFlag access)
{
    VulkanBuffer* VulknaBuffer = static_cast<VulkanBuffer*>(buffer.get());
    if (access == EDataAccessFlag::DA_WriteOnly)
    {
        VulknaBuffer->DynamicBufferIndex = (VulknaBuffer->DynamicBufferIndex+1) % 
    }
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
}

void FVulkanDynamicRHI::RHIUnlockBuffer(RHIBufferRef buffer)
{
    
}


}