#include "VulkanQueue.h"
#include "VulkanBuffer.h"
#include "VulkanDynamicRHI.h"
#include "VulkanSemaphore.h"
#include "VulkanCommandBuffer.h"

namespace nilou {

FVulkanQueue::FVulkanQueue(FVulkanDynamicRHI* InContext, VkDevice InDevice, uint32 InFamilyIndex)
    : FamilyIndex(InFamilyIndex)
    , QueueIndex(0)
    , Device(InDevice)
	, Context(InContext)
{
    vkGetDeviceQueue(InDevice, FamilyIndex, QueueIndex, &Handle);
}

void FVulkanQueue::Submit(FVulkanCmdBuffer* CmdBuffer, uint32 NumSignalSemaphores, VkSemaphore* SignalSemaphores)
{
	VkSubmitInfo SubmitInfo{};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CmdBuffer->Handle;
	SubmitInfo.signalSemaphoreCount = NumSignalSemaphores;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	std::vector<VkSemaphore> TempWaitSemas;
	if (CmdBuffer->WaitSemaphores.size() > 0)
	{
		TempWaitSemas.reserve(CmdBuffer->WaitSemaphores.size());
		for (auto& Sema : CmdBuffer->WaitSemaphores)
			TempWaitSemas.push_back(Sema->Handle);
		SubmitInfo.waitSemaphoreCount = TempWaitSemas.size();
		SubmitInfo.pWaitSemaphores = TempWaitSemas.data();
		SubmitInfo.pWaitDstStageMask = CmdBuffer->WaitFlags.data();
	}
    vkQueueSubmit(Handle, 1, &SubmitInfo, CmdBuffer->Fence);
	CmdBuffer->State = FVulkanCmdBuffer::EState::Submitted;
    CmdBuffer->MarkSemaphoresAsSubmitted();
	CmdBuffer->SubmittedFenceCounter = CmdBuffer->FenceSignaledCounter;
	LastSubmittedCmdBuffer = CmdBuffer;
	Context->StagingManager->ProcessPendingFree(false, false);
	// CmdBuffer->Wait(ULONG_MAX);
	// CmdBuffer->RefreshFenceStatus();
    
}

}