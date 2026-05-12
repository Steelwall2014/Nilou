#pragma once
#include "VulkanDynamicRHI.h"

#ifndef VULKAN_ENABLE_DRAW_MARKERS
#define VULKAN_ENABLE_DRAW_MARKERS 1
#endif

namespace nilou {

class VulkanQueue;
class VulkanCommandBufferPool;

class FVulkanPhysicalDeviceFeatures
{
public:
    FVulkanPhysicalDeviceFeatures()
    {
        std::memset(this, 0, sizeof(*this));
    }

    void Query(VkPhysicalDevice PhysicalDevice, uint32 APIVersion);

    VkPhysicalDeviceFeatures2        Features;
    VkPhysicalDeviceFeatures	     Core_1_0;
    VkPhysicalDeviceVulkan11Features Core_1_1;
private:
    // Anything above Core 1.1 cannot be assumed, they should only be used by the device at init time
    VkPhysicalDeviceVulkan12Features Core_1_2;
    VkPhysicalDeviceVulkan13Features Core_1_3;

    friend class FVulkanDevice;
};

class VulkanDevice
{
public:
    VulkanDevice(FVulkanDynamicRHI* InRHI, VkPhysicalDevice Gpu);
    ~VulkanDevice();

    void InitGPU();

#if VULKAN_ENABLE_DRAW_MARKERS
	struct
	{
		PFN_vkCmdBeginDebugUtilsLabelEXT	CmdBeginDebugUtilsLabel = nullptr;
		PFN_vkCmdEndDebugUtilsLabelEXT	CmdEndDebugUtilsLabel = nullptr;
		PFN_vkSetDebugUtilsObjectNameEXT	SetDebugName = nullptr;
	} DebugMarkers;

	void InitDebugMarkers();
	void SetDebugUtilsObjectName(VkObjectType ObjectType, uint64_t VulkanHandle, const char* Name);
#endif
	VkDevice Handle;
	VkPhysicalDevice Gpu;
	std::vector<VkQueueFamilyProperties> QueueFamilyProps;
    FVulkanPhysicalDeviceFeatures PhysicalDeviceFeatures;

	VulkanQueue* GfxQueue = nullptr;
	VulkanQueue* ComputeQueue = nullptr;
	VulkanQueue* TransferQueue = nullptr;
	VulkanQueue* PresentQueue = nullptr;

    VulkanCommandBufferPool* GfxCmdBufferPool = nullptr;
    VulkanCommandBufferPool* ComputeCmdBufferPool = nullptr;
    VulkanCommandBufferPool* TransferCmdBufferPool = nullptr;

    FVulkanMemoryManager* MemoryManager;

};

}