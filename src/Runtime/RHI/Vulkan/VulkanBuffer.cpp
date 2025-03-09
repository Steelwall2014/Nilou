#include "VulkanDevice.h"
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

template <typename T>
constexpr T AlignArbitrary(T Val, uint64 Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignArbitrary expects an integer or pointer type");

	return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
}

RHIBufferRef FVulkanDynamicRHI::RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data)
{
    VulkanBufferRef Buffer = new VulkanBuffer(Device, Stride, Size, InUsage);
    return Buffer;
}

RHIBufferRef FVulkanDynamicRHI::RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data)
{
    return RHICreateBuffer(DataByteLength, DataByteLength, EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::Dynamic, Data);
}

RHIBufferRef FVulkanDynamicRHI::RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z)
{
    VkDispatchIndirectCommand command{ num_groups_x, num_groups_y, num_groups_z };
    return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::DispatchIndirect | EBufferUsageFlags::Dynamic, &command);
}

RHIBufferRef FVulkanDynamicRHI::RHICreateDrawElementsIndirectBuffer(
    int32 Count, uint32 instanceCount, uint32 firstIndex, uint32 baseVertex, uint32 baseInstance)
{
    VkDrawIndexedIndirectCommand command{ (uint32)Count, instanceCount, firstIndex, (int32)baseVertex, baseInstance };
    return RHICreateBuffer(sizeof(command), sizeof(command), EBufferUsageFlags::DrawIndirect | EBufferUsageFlags::Dynamic, &command);
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
    void* MappedPointer = nullptr;
    vkMapMemory(Device->Handle, vkBuffer->Memory, Offset, Size, 0, &MappedPointer);
    return MappedPointer;
}

void FVulkanDynamicRHI::RHIUnmapMemory(RHIBuffer* buffer)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    vkUnmapMemory(Device->Handle, vkBuffer->Memory);
}

}