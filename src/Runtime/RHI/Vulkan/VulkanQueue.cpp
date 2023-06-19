#include "VulkanQueue.h"
#include "VulkanCommandBuffer.h"

namespace nilou {

FVulkanQueue::FVulkanQueue(VkDevice InDevice, uint32 InFamilyIndex)
    : FamilyIndex(InFamilyIndex)
    , QueueIndex(0)
    , Device(InDevice)
{
    vkGetDeviceQueue(InDevice, FamilyIndex, QueueIndex, &Queue);
}

void FVulkanQueue::Submit(FVulkanCmdBuffer* CmdBuffer, uint32 NumSignalSemaphores, VkSemaphore* SignalSemaphores)
{
	VkSubmitInfo SubmitInfo{};
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CmdBuffer->Handle;
	SubmitInfo.signalSemaphoreCount = NumSignalSemaphores;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	if (CmdBuffer->WaitSemaphores.size() > 0)
	{
		SubmitInfo.waitSemaphoreCount = CmdBuffer->WaitSemaphores.size();
		SubmitInfo.pWaitSemaphores = CmdBuffer->WaitSemaphores.data();
		SubmitInfo.pWaitDstStageMask = CmdBuffer->WaitFlags.data();
	}
    vkQueueSubmit(Queue, 1, &SubmitInfo, CmdBuffer->Fence);
	CmdBuffer->State = FVulkanCmdBuffer::EState::Submitted;
    CmdBuffer->MarkSemaphoresAsSubmitted();
	LastSubmittedCmdBuffer = CmdBuffer;
    
}

}