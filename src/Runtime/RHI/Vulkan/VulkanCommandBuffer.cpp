#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanQueue.h"
#include "VulkanDynamicRHI.h"
#include "VulkanTexture.h"
#include "VulkanBarriers.h"
#include "Common/Log.h"

namespace nilou {

FVulkanCmdBuffer::FVulkanCmdBuffer(FVulkanDynamicRHI* InContext, VkDevice InDevice, FVulkanCommandBufferPool* InCmdBufferPool, bool bInIsUploadOnly)
	: State(EState::NotAllocated)
	, Device(InDevice)
	, CmdBufferPool(InCmdBufferPool)
	, bIsUploadOnly(bInIsUploadOnly)
	, Context(InContext)
{
	AllocMemory();
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(Device, &fenceInfo, nullptr, &Fence);

}

FVulkanCmdBuffer::~FVulkanCmdBuffer()
{
	if (State != EState::NotAllocated)
	{
		FreeMemory();
	}
	if (State == EState::Submitted)
	{
		vkWaitForFences(Device, 1, &Fence, VK_TRUE, (uint64)(33 * 1000 * 1000LL));
	}
	vkDestroyFence(Device, Fence, nullptr);
}

void FVulkanCmdBuffer::AllocMemory()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = CmdBufferPool->Handle;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(Device, &allocInfo, &Handle);
	State = EState::ReadyForBegin;
}

void FVulkanCmdBuffer::FreeMemory()
{
	Ncheck(State != EState::NotAllocated);
	Ncheck(Handle != VK_NULL_HANDLE);
	vkFreeCommandBuffers(Device, CmdBufferPool->Handle, 1, &Handle);
	Handle = VK_NULL_HANDLE;
	State = EState::NotAllocated;
}

void FVulkanCmdBuffer::Begin()
{
	if(State == EState::NeedReset)
	{
		vkResetCommandBuffer(Handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}
	State = EState::IsInsideBegin;
	
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(Handle, &beginInfo);
}

void FVulkanCmdBuffer::End()
{
	vkEndCommandBuffer(Handle);
	State = EState::HasEnded;
}

void FVulkanCmdBuffer::BeginRenderPass(const FRHIRenderPassInfo& InInfo, VkRenderPass RenderPass, VkFramebuffer Framebuffer)
{

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = RenderPass;
	renderPassInfo.framebuffer = Framebuffer;
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = VkExtent2D{(uint32)InInfo.Viewport.x, (uint32)InInfo.Viewport.y};        
	
	FVulkanRenderTargetLayout RTLayout{InInfo};
	int32 DepthStencilAttachment = -1;
	if (RTLayout.bHasDepthAttachment)
		DepthStencilAttachment = RTLayout.DepthStencilReference.attachment;

	std::vector<VkClearValue> clearValues;
	for (int i = 0; i < RTLayout.Desc.size(); i++)
	{
		if (i == DepthStencilAttachment && (InInfo.bClearDepthBuffer || InInfo.bClearStencilBuffer))
		{
			VkClearValue& clearValue = clearValues.emplace_back();
			clearValue.depthStencil = {InInfo.ClearDepth, (uint32)InInfo.ClearStencil};
		}
		else if (InInfo.bClearColorBuffer)
		{
			VkClearValue& clearValue = clearValues.emplace_back();
			clearValue.color = { {InInfo.ClearColor.r, InInfo.ClearColor.g, InInfo.ClearColor.b, InInfo.ClearColor.a} };
		}
	}

	renderPassInfo.clearValueCount = static_cast<uint32>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	
	vkCmdBeginRenderPass(Handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	State = EState::IsInsideRenderPass;
}

FVulkanCommandBufferPool::~FVulkanCommandBufferPool()
{
	for (auto& CmdBuffer : CmdBuffers)
	{
		if (CmdBuffer->HasBegun())
		{
			if (!CmdBuffer->HasEnded())
			{
				CmdBuffer->End();
			}
			if (!CmdBuffer->IsSubmitted())
			{
				Mgr.Queue->Submit(CmdBuffer.get());
			}
		}
		Mgr.WaitForCmdBuffer(CmdBuffer.get());
	}
	CmdBuffers.clear();
	FreeCmdBuffers.clear();
	vkDestroyCommandPool(Device, Handle, nullptr);
	Handle = VK_NULL_HANDLE;
}

FVulkanCmdBuffer* FVulkanCommandBufferPool::Create(bool bIsUploadOnly)
{
	for (int32 Index = FreeCmdBuffers.size() - 1; Index >= 0; --Index)
	{
		std::shared_ptr<FVulkanCmdBuffer> CmdBuffer = FreeCmdBuffers[Index];
		if (CmdBuffer->bIsUploadOnly == bIsUploadOnly)
		{
			FreeCmdBuffers.erase(FreeCmdBuffers.begin()+Index);
			CmdBuffer->AllocMemory();
			CmdBuffers.push_back(CmdBuffer);
			return CmdBuffer.get();
		}
	}

	std::shared_ptr<FVulkanCmdBuffer> CmdBuffer = std::make_shared<FVulkanCmdBuffer>(Context, Device, this, bIsUploadOnly);
	CmdBuffers.push_back(CmdBuffer);
	return CmdBuffer.get();
}

void FVulkanCommandBufferPool::FreeUnusedCmdBuffers(FVulkanQueue* Queue)
{
	for (int32 Index = CmdBuffers.size() - 1; Index >= 0; --Index)
	{
		std::shared_ptr<FVulkanCmdBuffer> CmdBuffer = CmdBuffers[Index];
		if (CmdBuffer.get() != Queue->LastSubmittedCmdBuffer &&
			(CmdBuffer->State == FVulkanCmdBuffer::EState::ReadyForBegin || CmdBuffer->State == FVulkanCmdBuffer::EState::NeedReset))
		{
			CmdBuffer->FreeMemory();
			CmdBuffers.erase(CmdBuffers.begin()+Index);
			FreeCmdBuffers.push_back(CmdBuffer);
		}
	}
}

void FVulkanCommandBufferPool::RefreshFenceStatus(FVulkanCmdBuffer* SkipCmdBuffer)
{
	for (int32 Index = 0; Index < CmdBuffers.size(); ++Index)
	{
		FVulkanCmdBuffer* CmdBuffer = CmdBuffers[Index].get();
		if (CmdBuffer != SkipCmdBuffer)
		{
			CmdBuffer->RefreshFenceStatus();
		}
	}
}

void FVulkanCmdBuffer::RefreshFenceStatus()
{
	if (State == EState::Submitted)
	{
		if (vkGetFenceStatus(Device, Fence) == VK_SUCCESS)
		{
			SubmittedWaitSemaphores.clear();
			vkResetFences(Device, 1, &Fence);
			++FenceSignaledCounter;
			if (CurrentSetContainer)
			{
				Context->DescriptorPoolsManager->ReleasePoolSet(CurrentSetContainer);
				CurrentSetContainer = nullptr;
			}
			State = EState::NeedReset;
		}
	}
}

FVulkanCommandBufferManager::FVulkanCommandBufferManager(VkDevice InDevice, FVulkanDynamicRHI* InContext)
    : Queue(InContext->GfxQueue.get())
	, Pool(new FVulkanCommandBufferPool(InContext, InDevice, *this, InContext->GfxQueue->FamilyIndex))
	, Device(InDevice)
	, Context(InContext)
{
	ActiveCmdBufferSemaphore = CreateSemephore(Device);
	ActiveCmdBuffer = Pool->Create(false);
}

FVulkanCommandBufferManager::~FVulkanCommandBufferManager()
{
	// The Pool must be release before semaphores
	Pool = nullptr;
	ActiveCmdBufferSemaphore = nullptr;
	UploadCmdBufferSemaphore = nullptr;
	RenderingCompletedSemaphores.clear();
	UploadCompletedSemaphores.clear();
}

FVulkanCmdBuffer* FVulkanCommandBufferManager::GetUploadCmdBuffer()
{
	if (!UploadCmdBuffer)
	{
		UploadCmdBufferSemaphore = CreateSemephore(Device);
		for (int32 Index = 0; Index < Pool->CmdBuffers.size(); ++Index)
		{
			std::shared_ptr<FVulkanCmdBuffer> CmdBuffer = Pool->CmdBuffers[Index];
			CmdBuffer->RefreshFenceStatus();
			if (CmdBuffer->bIsUploadOnly)
			{
				if (CmdBuffer->State == FVulkanCmdBuffer::EState::ReadyForBegin || CmdBuffer->State == FVulkanCmdBuffer::EState::NeedReset)
				{
					UploadCmdBuffer = CmdBuffer.get();
					UploadCmdBuffer->Begin();
					return UploadCmdBuffer;
				}
			}
		}

		// All cmd buffers are being executed still
		UploadCmdBuffer = Pool->Create(true);
		UploadCmdBuffer->Begin();
	}

	return UploadCmdBuffer;
}

void FVulkanCommandBufferManager::SubmitUploadCmdBuffer(uint32 NumSignalSemaphores, VkSemaphore* SignalSemaphores)
{
	if (!UploadCmdBuffer->IsSubmitted() && UploadCmdBuffer->HasBegun())
    {
		UploadCmdBuffer->End();

		// Add semaphores associated with the recent active cmdbuf(s), if any. That will prevent
		// the overlap, delaying execution of this cmdbuf until the graphics one(s) is complete.
		for (std::shared_ptr<FVulkanSemaphore> WaitForThis : RenderingCompletedSemaphores)
		{
			UploadCmdBuffer->AddWaitSemaphore(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, WaitForThis);
		}

		if (NumSignalSemaphores == 0)
		{
			Ncheck(UploadCmdBufferSemaphore != nullptr);
			std::shared_ptr<FVulkanSemaphore> Sema = UploadCmdBufferSemaphore;
			Queue->Submit(UploadCmdBuffer, 1, &Sema->Handle);
			UploadCompletedSemaphores.push_back(UploadCmdBufferSemaphore);
			UploadCmdBufferSemaphore = nullptr;
		}
		else
		{
			std::vector<VkSemaphore> CombinedSemaphores;
			CombinedSemaphores.push_back(UploadCmdBufferSemaphore->Handle);
			for (int i = 0; i < NumSignalSemaphores; i++)
				CombinedSemaphores.push_back(SignalSemaphores[i]);
			Queue->Submit(UploadCmdBuffer, CombinedSemaphores.size(), CombinedSemaphores.data());
		}
		// the buffer will now hold on to the wait semaphores, so we can clear them here
		RenderingCompletedSemaphores.clear();
    }
    UploadCmdBuffer = nullptr;
}

void FVulkanCommandBufferManager::SubmitActiveCmdBuffer(std::vector<VkSemaphore> SignalSemaphores)
{
	if (!ActiveCmdBuffer->IsSubmitted() && ActiveCmdBuffer->HasBegun())
	{
		if (!ActiveCmdBuffer->IsOutsideRenderPass())
		{
			NILOU_LOG(Warning, "Forcing EndRenderPass() for submission");
			ActiveCmdBuffer->EndRenderPass();
		}

		ActiveCmdBuffer->End();
		// Add semaphores associated with the recent upload cmdbuf(s), if any. That will prevent
		// the overlap, delaying execution of this cmdbuf until upload one(s) are complete.
		for (std::shared_ptr<FVulkanSemaphore> UploadCompleteSema : UploadCompletedSemaphores)
		{
			ActiveCmdBuffer->AddWaitSemaphore(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, UploadCompleteSema);
		}

		SignalSemaphores.push_back(ActiveCmdBufferSemaphore->Handle);
		Queue->Submit(ActiveCmdBuffer, SignalSemaphores.size(), SignalSemaphores.data());

		RenderingCompletedSemaphores.push_back(ActiveCmdBufferSemaphore);
		ActiveCmdBufferSemaphore = nullptr;

		// the buffer will now hold on to the wait semaphores, so we can clear them here
		UploadCompletedSemaphores.clear();	
	
	}

	ActiveCmdBuffer = nullptr;

}

void FVulkanCommandBufferManager::FreeUnusedCmdBuffers()
{
	Pool->FreeUnusedCmdBuffers(Queue);
}

void FVulkanCommandBufferManager::PrepareForNewActiveCommandBuffer()
{
	if (ActiveCmdBufferSemaphore == nullptr)
		ActiveCmdBufferSemaphore = CreateSemephore(Device);
	for (int32 Index = 0; Index < Pool->CmdBuffers.size(); ++Index)
	{
		std::shared_ptr<FVulkanCmdBuffer> CmdBuffer = Pool->CmdBuffers[Index];
		CmdBuffer->RefreshFenceStatus();
		if (!CmdBuffer->bIsUploadOnly)
		{
			if (CmdBuffer->State == FVulkanCmdBuffer::EState::ReadyForBegin || CmdBuffer->State == FVulkanCmdBuffer::EState::NeedReset)
			{
				ActiveCmdBuffer = CmdBuffer.get();
				ActiveCmdBuffer->Begin();
				return;
			}
			else
			{
				Ncheck(CmdBuffer->State == FVulkanCmdBuffer::EState::Submitted);
			}
		}
	}

	// All cmd buffers are being executed still
	ActiveCmdBuffer = Pool->Create(false);
	ActiveCmdBuffer->Begin();
}

void FVulkanCommandBufferManager::WaitForCmdBuffer(FVulkanCmdBuffer* CmdBuffer, float TimeInSecondsToWait)
{
	CmdBuffer->Wait(TimeInSecondsToWait);
}

}