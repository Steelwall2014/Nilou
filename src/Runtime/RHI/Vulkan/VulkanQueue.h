#pragma once

#include <vulkan/vulkan.h>
#include "Platform.h"

namespace nilou {

class FVulkanCmdBuffer;
class FVulkanDynamicRHI;

class FVulkanQueue
{
public:

    FVulkanQueue(FVulkanDynamicRHI* InContext, VkDevice InDevice, uint32 InFamilyIndex);

	void Submit(FVulkanCmdBuffer* CmdBuffer, uint32 NumSignalSemaphores = 0, VkSemaphore* SignalSemaphores = nullptr);

	inline void Submit(FVulkanCmdBuffer* CmdBuffer, VkSemaphore SignalSemaphore)
	{
		Submit(CmdBuffer, 1, &SignalSemaphore);
	}

	FVulkanDynamicRHI* Context;
    VkDevice Device;
    VkQueue Handle;
	uint32 FamilyIndex;
	uint32 QueueIndex;

	FVulkanCmdBuffer* LastSubmittedCmdBuffer;
};

}