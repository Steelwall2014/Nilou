#include "VulkanBuffer.h"
#include "VulkanDynamicRHI.h"
#include "Common/EnumClassFlags.h"

namespace nilou {

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

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Device, *Buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(Device, &allocInfo, nullptr, Memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

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


}