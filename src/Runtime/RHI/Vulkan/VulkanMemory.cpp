#include "VulkanMemory.h"
#include <stdexcept>

namespace nilou {

void FVulkanMemoryManager::AllocateBufferMemory(VkDeviceMemory* Memory, VkBuffer Buffer, VkMemoryPropertyFlags MemPropertyFlags)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Device, Buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, MemPropertyFlags);

    if (vkAllocateMemory(Device, &allocInfo, nullptr, Memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
}

void FVulkanMemoryManager::AllocateImageMemory(VkDeviceMemory* Memory, VkImage Image, VkMemoryPropertyFlags MemPropertyFlags)
{
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Device, Image, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, MemPropertyFlags);

    if (vkAllocateMemory(Device, &allocInfo, nullptr, Memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
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