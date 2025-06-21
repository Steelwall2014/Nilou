#pragma once

#include <vulkan/vulkan.h>
#include "Platform.h"

namespace nilou {

class VulkanQueue
{
public:
    VulkanQueue(VkDevice InDevice, uint32 InFamilyIndex)
		: Device(InDevice)
		, FamilyIndex(InFamilyIndex)
	{
		vkGetDeviceQueue(InDevice, FamilyIndex, QueueIndex, &Handle);
	}
    VkDevice Device;
    VkQueue Handle;
	uint32 FamilyIndex;
	uint32 QueueIndex = 0;
};

}