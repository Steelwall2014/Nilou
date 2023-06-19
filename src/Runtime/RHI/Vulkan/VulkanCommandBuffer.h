#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Platform.h"
#include "RHIResources.h"

namespace nilou {

class FVulkanQueue;
class FVulkanCommandBufferPool;
class FVulkanCommandBufferManager;

class FVulkanCmdBuffer
{
public:

    friend class FVulkanQueue;

	FVulkanCmdBuffer(VkDevice InDevice, FVulkanCommandBufferPool* CmdBufferPool, bool bIsUploadOnly);
	~FVulkanCmdBuffer();

	bool IsInsideRenderPass() const
	{
		return State == EState::IsInsideRenderPass;
	}

	bool IsOutsideRenderPass() const
	{
		return State == EState::IsInsideBegin;
	}

	bool HasBegun() const
	{
		return State == EState::IsInsideBegin || State == EState::IsInsideRenderPass;
	}

	bool HasEnded() const
	{
		return State == EState::HasEnded;
	}

	bool IsSubmitted() const
	{
		return State == EState::Submitted;
	}

	bool IsAllocated() const
	{
		return State != EState::NotAllocated;
	}

	void AllocMemory();
	void FreeMemory();

    VkCommandBuffer Handle{};

    std::vector<VkSemaphore> WaitSemaphores;
	std::vector<VkSemaphore> SubmittedWaitSemaphores;

    std::vector<VkPipelineStageFlags> WaitFlags;

	VkDevice Device{};

	VkFence Fence{};

	FVulkanCommandBufferPool* CmdBufferPool;

    void Begin();
    void End();
	
	void BeginRenderPass(const FRHIRenderPassInfo& InInfo, VkRenderPass RenderPass, VkFramebuffer Framebuffer);
	void EndRenderPass()
	{
		vkCmdEndRenderPass(Handle);
		State = EState::IsInsideBegin;
	}

    bool bIsUploadOnly;
    
	enum class EState : uint8
	{
		ReadyForBegin,
		IsInsideBegin,
		IsInsideRenderPass,
		HasEnded,
		Submitted,
		NotAllocated,
		NeedReset,
	};
	EState State;

private:

	void MarkSemaphoresAsSubmitted()
	{
		WaitFlags.clear();
		// Move to pending delete list
		SubmittedWaitSemaphores = WaitSemaphores;
		WaitSemaphores.clear();
	}

};

class FVulkanCommandBufferPool
{
public:

    FVulkanCommandBufferPool(VkDevice InDevice, FVulkanCommandBufferManager& InMgr);

    VkCommandPool Handle;

    VkDevice Device;

	std::vector<FVulkanCmdBuffer*> CmdBuffers;
	std::vector<FVulkanCmdBuffer*> FreeCmdBuffers;

    FVulkanCommandBufferManager& Mgr;

    FVulkanCmdBuffer* Create(bool bIsUploadOnly);

	void FreeUnusedCmdBuffers(FVulkanQueue* Queue);
};

class FVulkanCommandBufferManager
{
public:

    FVulkanCommandBufferManager(VkDevice InDevice);

	FVulkanCmdBuffer* GetActiveCmdBuffer()
    {
        if (UploadCmdBuffer)
        {
            SubmitUploadCmdBuffer();
        }

		return ActiveCmdBuffer;
    }

	FVulkanCmdBuffer* GetUploadCmdBuffer();

	void SubmitUploadCmdBuffer(uint32 NumSignalSemaphores = 0, VkSemaphore* SignalSemaphores = nullptr);

	void SubmitActiveCmdBuffer(std::vector<VkSemaphore> SignalSemaphores);

	void FreeUnusedCmdBuffers();

    FVulkanCommandBufferPool Pool;

    FVulkanQueue* Queue;

    FVulkanCmdBuffer* ActiveCmdBuffer;
    FVulkanCmdBuffer* UploadCmdBuffer;

};

}
