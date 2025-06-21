#include <stdexcept>
#include "VulkanMemory.h"
#include "VulkanDynamicRHI.h"

namespace nilou {

constexpr uint32 FVulkanMemoryManager::PoolSizes[(int32)FVulkanMemoryManager::EPoolSizes::SizesCount];
constexpr uint32 FVulkanMemoryManager::BufferSizes[(int32)FVulkanMemoryManager::EPoolSizes::SizesCount + 1];

template<typename BitsType>
constexpr bool VKHasAnyFlags(VkFlags Flags, BitsType Contains)
{
	return (Flags & Contains) != 0;
}
static uint32 CalculateBufferAlignmentFromVKUsageFlags(FVulkanDynamicRHI* Context, const VkBufferUsageFlags BufferUsageFlags)
{
    const VkPhysicalDeviceLimits& Limits = Context->GpuProps.limits;

    const bool bIsTexelBuffer = VKHasAnyFlags(BufferUsageFlags, (VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT));
    const bool bIsStorageBuffer = VKHasAnyFlags(BufferUsageFlags, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    const bool bIsVertexOrIndexBuffer = VKHasAnyFlags(BufferUsageFlags, (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
    const bool bIsAccelerationStructureBuffer = VKHasAnyFlags(BufferUsageFlags, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    const bool bIsUniformBuffer = VKHasAnyFlags(BufferUsageFlags, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    uint32 Alignment = 1;

    if (bIsTexelBuffer || bIsStorageBuffer)
    {
        Alignment = std::max(Alignment, (uint32)Limits.minTexelBufferOffsetAlignment);
        Alignment = std::max(Alignment, (uint32)Limits.minStorageBufferOffsetAlignment);
    }
    else if (bIsVertexOrIndexBuffer)
    {
        // No alignment restrictions on Vertex or Index buffers, leave it at 1
    }
    else if (bIsAccelerationStructureBuffer)
    {
        Alignment = std::max(Alignment, (uint32)256);
    }
    else if (bIsUniformBuffer)
    {
        Alignment = std::max(Alignment, (uint32)Limits.minUniformBufferOffsetAlignment);
    }
    else
    {
        
    }

    return Alignment;
}


void FVulkanMemoryManager::AllocateBufferMemory(VkDeviceMemory* Memory, VkBuffer Buffer, VkMemoryPropertyFlags MemPropertyFlags)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Device, Buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, MemPropertyFlags);

    VK_CHECK_RESULT(vkAllocateMemory(Device, &allocInfo, nullptr, Memory));
}

void FVulkanMemoryManager::AllocateImageMemory(VkDeviceMemory* Memory, VkImage Image, VkMemoryPropertyFlags MemPropertyFlags)
{
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Device, Image, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, MemPropertyFlags);

    VK_CHECK_RESULT(vkAllocateMemory(Device, &allocInfo, nullptr, Memory));
}

void FVulkanMemoryManager::AllocateBufferPooled(VkDeviceMemory* Memory, uint32 Size, VkMemoryPropertyFlags MemPropertyFlags)
{
    
}

void FVulkanMemoryManager::FreeMemory(VkDeviceMemory Memory)
{
    vkFreeMemory(Device, Memory, nullptr);
}

uint32 FVulkanMemoryManager::findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(PhysDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

}