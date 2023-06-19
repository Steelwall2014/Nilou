#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "Common/Log.h"

namespace nilou {

FVulkanCmdBuffer::FVulkanCmdBuffer(VkDevice InDevice, FVulkanCommandBufferPool* InCmdBufferPool, bool bIsUploadOnly)
	: State(EState::NotAllocated)
	, Device(InDevice)
	, CmdBufferPool(InCmdBufferPool)
{
	AllocMemory();
}

FVulkanCmdBuffer::~FVulkanCmdBuffer()
{
	if (State != EState::NotAllocated)
	{
		FreeMemory();
	}
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
	vkFreeCommandBuffers(Device, CmdBufferPool->Handle, 1, &Handle);
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
	
	std::vector<VkClearValue> clearValues;
	if (InInfo.bClearColorBuffer)
	{
		VkClearValue& clearValue = clearValues.emplace_back();
		clearValue.color = { {InInfo.ClearColor.r, InInfo.ClearColor.g, InInfo.ClearColor.b, InInfo.ClearColor.a} };
	}
	if (InInfo.bClearDepthBuffer || InInfo.bClearStencilBuffer)
	{
		VkClearValue& clearValue = clearValues.emplace_back();
		clearValue.depthStencil = {InInfo.ClearDepth, (uint32)InInfo.ClearStencil};
	}

	renderPassInfo.clearValueCount = static_cast<uint32>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	
	vkCmdBeginRenderPass(Handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	State = EState::IsInsideRenderPass;
}

FVulkanCmdBuffer* FVulkanCommandBufferPool::Create(bool bIsUploadOnly)
{
	for (int32 Index = FreeCmdBuffers.size() - 1; Index >= 0; --Index)
	{
		FVulkanCmdBuffer* CmdBuffer = FreeCmdBuffers[Index];
		if (CmdBuffer->bIsUploadOnly == bIsUploadOnly)
		{
			FreeCmdBuffers.erase(FreeCmdBuffers.begin()+Index);
			CmdBuffer->AllocMemory();
			CmdBuffers.push_back(CmdBuffer);
			return CmdBuffer;
		}
	}

	FVulkanCmdBuffer* CmdBuffer = new FVulkanCmdBuffer(Device, this, bIsUploadOnly);
	CmdBuffers.push_back(CmdBuffer);
}

void FVulkanCommandBufferPool::FreeUnusedCmdBuffers(FVulkanQueue* Queue)
{
	for (int32 Index = CmdBuffers.size() - 1; Index >= 0; --Index)
	{
		FVulkanCmdBuffer* CmdBuffer = CmdBuffers[Index];
		if (CmdBuffer != Queue->LastSubmittedCmdBuffer &&
			(CmdBuffer->State == FVulkanCmdBuffer::EState::ReadyForBegin || CmdBuffer->State == FVulkanCmdBuffer::EState::NeedReset))
		{
			CmdBuffer->FreeMemory();
			CmdBuffers.erase(CmdBuffers.begin()+Index);
			FreeCmdBuffers.push_back(CmdBuffer);
		}
	}
}

FVulkanCommandBufferManager::FVulkanCommandBufferManager(VkDevice InDevice)
    : Pool(InDevice, *this)
{

}

FVulkanCmdBuffer* FVulkanCommandBufferManager::GetUploadCmdBuffer()
{
	if (!UploadCmdBuffer)
	{
		for (int32 Index = 0; Index < Pool.CmdBuffers.size(); ++Index)
		{
			FVulkanCmdBuffer* CmdBuffer = Pool.CmdBuffers[Index];
			if (CmdBuffer->bIsUploadOnly)
			{
                UploadCmdBuffer = CmdBuffer;
                UploadCmdBuffer->Begin();
                return UploadCmdBuffer;
			}
		}

		// All cmd buffers are being executed still
		UploadCmdBuffer = Pool.Create(true);
		UploadCmdBuffer->Begin();
	}

	return UploadCmdBuffer;
}

void FVulkanCommandBufferManager::SubmitUploadCmdBuffer(uint32 NumSignalSemaphores, VkSemaphore* SignalSemaphores)
{
	if (!UploadCmdBuffer->IsSubmitted() && UploadCmdBuffer->HasBegun())
    {
		UploadCmdBuffer->End();
        Queue->Submit(UploadCmdBuffer, NumSignalSemaphores, SignalSemaphores);
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
		Queue->Submit(ActiveCmdBuffer, SignalSemaphores.size(), SignalSemaphores.data());
	}

	ActiveCmdBuffer = nullptr;

}

void FVulkanCommandBufferManager::FreeUnusedCmdBuffers()
{
	Pool.FreeUnusedCmdBuffers(Queue);
}

}