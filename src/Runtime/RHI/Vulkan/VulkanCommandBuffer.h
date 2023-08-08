#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Platform.h"
#include "RHIResources.h"
#include "Common/Log.h"

namespace nilou {

class FVulkanQueue;
class FVulkanCommandBufferPool;
class FVulkanCommandBufferManager;
class FVulkanSemaphore;

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

	void Wait(uint64 Time)
	{
		vkWaitForFences(Device, 1, &Fence, VK_TRUE, Time);
		RefreshFenceStatus();
	}

	void AllocMemory();
	void FreeMemory();

	VkCommandBuffer GetHandle() const { return Handle; }

	void RefreshFenceStatus();

	void AddWaitSemaphore(VkPipelineStageFlags Flags, std::shared_ptr<FVulkanSemaphore> Semaphore)
	{
		Ncheck(std::find(WaitSemaphores.begin(), WaitSemaphores.end(), Semaphore) == WaitSemaphores.end());
		WaitSemaphores.push_back(Semaphore);
		WaitFlags.push_back(Flags);
	}

	uint64 GetFenceSignaledCounter() const
	{
		return FenceSignaledCounter;
	}

    VkCommandBuffer Handle{};

    std::vector<std::shared_ptr<FVulkanSemaphore>> WaitSemaphores;
	std::vector<std::shared_ptr<FVulkanSemaphore>> SubmittedWaitSemaphores;

    std::vector<VkPipelineStageFlags> WaitFlags;

	VkDevice Device{};

	VkFence Fence{};

	// Last value passed after the fence got signaled
	volatile uint64 FenceSignaledCounter{};
	// Last value when we submitted the cmd buffer; useful to track down if something waiting for the fence has actually been submitted
	volatile uint64 SubmittedFenceCounter{};

	FVulkanCommandBufferPool* CmdBufferPool{};
	
	class FVulkanDescriptorPoolSetContainer* CurrentSetContainer{};

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

    FVulkanCommandBufferPool(VkDevice InDevice, FVulkanCommandBufferManager& InMgr, int32 QueueFamilyIndex)
		: Device(InDevice)
		, Mgr(InMgr)
	{
		VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = QueueFamilyIndex;
		vkCreateCommandPool(Device, &poolInfo, nullptr, &Handle);
	}
	~FVulkanCommandBufferPool();

    VkCommandPool Handle{};

    VkDevice Device;

	std::vector<std::shared_ptr<FVulkanCmdBuffer>> CmdBuffers;
	std::vector<std::shared_ptr<FVulkanCmdBuffer>> FreeCmdBuffers;

    FVulkanCommandBufferManager& Mgr;

    FVulkanCmdBuffer* Create(bool bIsUploadOnly);

	void FreeUnusedCmdBuffers(FVulkanQueue* Queue);

	void RefreshFenceStatus(FVulkanCmdBuffer* SkipCmdBuffer = nullptr);
};

class FVulkanCommandBufferManager
{
public:

    FVulkanCommandBufferManager(VkDevice InDevice, class FVulkanDynamicRHI* Context);

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

	void PrepareForNewActiveCommandBuffer();

	bool HasPendingUploadCmdBuffer() const
	{
		return UploadCmdBuffer != nullptr;
	}

	bool HasPendingActiveCmdBuffer() const
	{
		return ActiveCmdBuffer != nullptr;
	}

	void WaitForCmdBuffer(FVulkanCmdBuffer* CmdBuffer, float TimeInSecondsToWait = 10.0f);

	void RefreshFenceStatus(FVulkanCmdBuffer* SkipCmdBuffer = nullptr)
	{
		Pool.RefreshFenceStatus(SkipCmdBuffer);
	}

	VkDevice Device;

    FVulkanCommandBufferPool Pool;

    FVulkanQueue* Queue{};

    FVulkanCmdBuffer* ActiveCmdBuffer = nullptr;
    FVulkanCmdBuffer* UploadCmdBuffer = nullptr;

	/** This semaphore is used to prevent overlaps between the (current) graphics cmdbuf and next upload cmdbuf. */
	std::shared_ptr<FVulkanSemaphore> ActiveCmdBufferSemaphore{};

	/** Holds semaphores associated with the recent render cmdbuf(s) - waiting to be added to the next graphics cmdbuf as WaitSemaphores. */
	std::vector<std::shared_ptr<FVulkanSemaphore>> RenderingCompletedSemaphores;
	
	/** This semaphore is used to prevent overlaps between (current) upload cmdbuf and next graphics cmdbuf. */
	std::shared_ptr<FVulkanSemaphore> UploadCmdBufferSemaphore{};
	
	/** Holds semaphores associated with the recent upload cmdbuf(s) - waiting to be added to the next graphics cmdbuf as WaitSemaphores. */
	std::vector<std::shared_ptr<FVulkanSemaphore>> UploadCompletedSemaphores;

};

}
