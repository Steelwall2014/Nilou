#pragma once

#include <vulkan/vulkan.h>
#include "Platform.h"

namespace nilou {

class FVulkanMemoryManager
{
public:
    FVulkanMemoryManager(VkDevice InDevice, VkPhysicalDevice InPhysDevice)
        : Device(InDevice)
        , PhysDevice(InPhysDevice)
    { }
    VkDevice Device;
    VkPhysicalDevice PhysDevice;
    void AllocateBufferMemory(VkDeviceMemory* Memory, VkBuffer Buffer, VkMemoryPropertyFlags MemPropertyFlags);
    void AllocateImageMemory(VkDeviceMemory* Memory, VkImage Image, VkMemoryPropertyFlags MemPropertyFlags);
    void AllocateBufferPooled(VkDeviceMemory* Memory, uint32 Size, VkMemoryPropertyFlags MemPropertyFlags);
    void FreeMemory(VkDeviceMemory Memory);

protected:

    // pool sizes that we support
    enum class EPoolSizes : uint8
    {
// 		E32,
// 		E64,
        E128,
        E256,
        E512,
        E1k,
        E2k,
        E8k,
        E16k,
        SizesCount,
    };


    constexpr static uint32 PoolSizes[(int32)EPoolSizes::SizesCount] =
    {
// 			32,
// 			64,
        128,
        256,
        512,
        1024,
        2048,
        8192,
        16 * 1024,
    };

    constexpr static uint32 BufferSizes[(int32)EPoolSizes::SizesCount + 1] =
    {
// 			64 * 1024,
// 			64 * 1024,
        128 * 1024,
        128 * 1024,
        256 * 1024,
        256 * 1024,
        512 * 1024,
        512 * 1024,
        1024 * 1024,
        1 * 1024 * 1024,
    };


    EPoolSizes GetPoolTypeForAlloc(uint32 Size)
    {
        EPoolSizes PoolSize = EPoolSizes::SizesCount;
        for (int32 i = 0; i < (int32)EPoolSizes::SizesCount; ++i)
        {
            if (PoolSizes[i] >= Size)
            {
                PoolSize = (EPoolSizes)i;
                break;
            }
        }
        return PoolSize;
    }

private:
    uint32 findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);
};

}