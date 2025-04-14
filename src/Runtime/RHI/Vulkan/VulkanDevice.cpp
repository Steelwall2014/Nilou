#include "VulkanDevice.h"
#include "VulkanQueue.h"
#include "VulkanMemory.h"
#include "VulkanCommandBuffer.h"

namespace nilou {

void FVulkanPhysicalDeviceFeatures::Query(VkPhysicalDevice PhysicalDevice, uint32 APIVersion)
{
	VkPhysicalDeviceFeatures2 PhysicalDeviceFeatures2{};
    PhysicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

	PhysicalDeviceFeatures2.pNext = &Core_1_1;
	Core_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

	if (APIVersion >= VK_API_VERSION_1_2)
	{
		Core_1_1.pNext = &Core_1_2;
		Core_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	}

	if (APIVersion >= VK_API_VERSION_1_3)
	{
		Core_1_2.pNext = &Core_1_3;
		Core_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	}

	vkGetPhysicalDeviceFeatures2(PhysicalDevice, &PhysicalDeviceFeatures2);

	// Copy features into old struct for convenience
	Core_1_0 = PhysicalDeviceFeatures2.features;

	// Apply config modifications
	Core_1_0.robustBufferAccess = VK_TRUE;

	// Apply platform restrictions
	// Disable everything sparse-related
	Core_1_0.shaderResourceResidency = VK_FALSE;
	Core_1_0.shaderResourceMinLod = VK_FALSE;
	Core_1_0.sparseBinding = VK_FALSE;
	Core_1_0.sparseResidencyBuffer = VK_FALSE;
	Core_1_0.sparseResidencyImage2D = VK_FALSE;
	Core_1_0.sparseResidencyImage3D = VK_FALSE;
	Core_1_0.sparseResidency2Samples = VK_FALSE;
	Core_1_0.sparseResidency4Samples = VK_FALSE;
	Core_1_0.sparseResidency8Samples = VK_FALSE;
	Core_1_0.sparseResidencyAliased = VK_FALSE;
}

VulkanDevice::VulkanDevice(FVulkanDynamicRHI* InRHI, VkPhysicalDevice InGpu)
    : Gpu(InGpu)
{
	InitGPU();
}

void VulkanDevice::InitGPU()
{
    uint32 QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &QueueFamilyCount, nullptr);
    QueueFamilyProps.resize(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &QueueFamilyCount, QueueFamilyProps.data());

	// Query base features
    PhysicalDeviceFeatures.Query(Gpu, VK_API_VERSION_1_3);
	std::vector<VkDeviceQueueCreateInfo> QueueFamilyInfos;
	int32 GfxQueueFamilyIndex = -1;
	int32 ComputeQueueFamilyIndex = -1;
	int32 TransferQueueFamilyIndex = -1;
	NILOU_LOG(Display, "Found {} Queue Families", QueueFamilyProps.size());
	uint32 NumPriorities = 0;
	for (int32 FamilyIndex = 0; FamilyIndex < QueueFamilyProps.size(); ++FamilyIndex)
	{
		const VkQueueFamilyProperties& CurrProps = QueueFamilyProps[FamilyIndex];

		bool bIsValidQueue = false;
		if ((CurrProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			if (GfxQueueFamilyIndex == -1)
			{
				GfxQueueFamilyIndex = FamilyIndex;
				bIsValidQueue = true;
			}
			else
			{
				//#todo-rco: Support for multi-queue/choose the best queue!
			}
		}

		if ((CurrProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
		{
			if (ComputeQueueFamilyIndex == -1 && GfxQueueFamilyIndex != FamilyIndex)
			{
				ComputeQueueFamilyIndex = FamilyIndex;
				bIsValidQueue = true;
			}
		}

		if ((CurrProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
		{
			// Prefer a non-gfx transfer queue
			if (TransferQueueFamilyIndex == -1 && (CurrProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT && (CurrProps.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT)
			{
				TransferQueueFamilyIndex = FamilyIndex;
				bIsValidQueue = true;
			}
		}

		if (!bIsValidQueue)
		{
			NILOU_LOG(Display,"Skipping unnecessary Queue Family {}: {}", FamilyIndex, CurrProps.queueCount);
			continue;
		}

		VkDeviceQueueCreateInfo CurrQueue{};
		CurrQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		CurrQueue.queueFamilyIndex = FamilyIndex;
		CurrQueue.queueCount = CurrProps.queueCount;
		NumPriorities += CurrProps.queueCount;
        QueueFamilyInfos.push_back(CurrQueue);
		NILOU_LOG(Display, "Initializing Queue Family {}: {}", FamilyIndex, CurrProps.queueCount);
	}

	std::vector<float> QueuePriorities;
	QueuePriorities.resize(NumPriorities);
	float* CurrentPriority = QueuePriorities.data();
	for (int32 Index = 0; Index < QueueFamilyInfos.size(); ++Index)
	{
		VkDeviceQueueCreateInfo& CurrQueue = QueueFamilyInfos[Index];
		CurrQueue.pQueuePriorities = CurrentPriority;

		const VkQueueFamilyProperties& CurrProps = QueueFamilyProps[CurrQueue.queueFamilyIndex];
		for (int32 QueueIndex = 0; QueueIndex < (int32)CurrProps.queueCount; ++QueueIndex)
		{
			*CurrentPriority++ = 1.0f;
		}
	}
    
	std::vector<const char*> DeviceExtensions = {
		"VK_KHR_swapchain",
		VK_EXT_NON_SEAMLESS_CUBE_MAP_EXTENSION_NAME
	};
    std::vector<const char*> Layers = {
        "VK_LAYER_KHRONOS_validation"
    };
    VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = QueueFamilyInfos.size();
	createInfo.pQueueCreateInfos = QueueFamilyInfos.data();
	createInfo.enabledExtensionCount = DeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = DeviceExtensions.data();
	createInfo.enabledLayerCount = Layers.size();
	createInfo.ppEnabledLayerNames = Layers.data();
	createInfo.pEnabledFeatures = &PhysicalDeviceFeatures.Core_1_0;
	VK_CHECK_RESULT(vkCreateDevice(Gpu, &createInfo, nullptr, &Handle));
	NILOU_LOG(Display, "Create logical device")

	// Create Graphics Queue, here we submit command buffers for execution
	GfxQueue = new VulkanQueue(Handle, GfxQueueFamilyIndex);
	if (ComputeQueueFamilyIndex == -1)
	{
		// If we didn't find a dedicated Queue, use the default one
		ComputeQueueFamilyIndex = GfxQueueFamilyIndex;
	}
	ComputeQueue = new VulkanQueue(Handle, ComputeQueueFamilyIndex);
	if (TransferQueueFamilyIndex == -1)
	{
		// If we didn't find a dedicated Queue, use the default one
		TransferQueueFamilyIndex = ComputeQueueFamilyIndex;
	}
	TransferQueue = new VulkanQueue(Handle, TransferQueueFamilyIndex);
	PresentQueue = GfxQueue;

	GfxCmdBufferPool = new VulkanCommandBufferPool(Handle, GfxQueue->Handle, GfxQueueFamilyIndex);
	ComputeCmdBufferPool = new VulkanCommandBufferPool(Handle, ComputeQueue->Handle, ComputeQueueFamilyIndex);
	TransferCmdBufferPool = new VulkanCommandBufferPool(Handle, TransferQueue->Handle, TransferQueueFamilyIndex);

	MemoryManager = new FVulkanMemoryManager(Handle, Gpu);
}

}