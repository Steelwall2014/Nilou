#pragma once
#include <vulkan/vulkan.h>
#include "RHICommandList.h"

namespace nilou {

    class VulkanQueue;

    class VulkanCommandBuffer : public RHICommandList
    {
    public:
        VulkanCommandBuffer(VkDevice Device, VkQueue Queue, VkCommandPool Pool);
        ~VulkanCommandBuffer();

        /* Perform actions commands */
        virtual void BeginRenderPass(FRHIRenderPassInfo& Info) override;
        virtual void EndRenderPass() override;
        virtual void DrawArrays(uint32 VertexCount, uint32 InstanceCount, uint32 FirstVertex, uint32 FirstInstance) override;
        virtual void DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, uint32 VertexOffset, uint32 FirstInstance) override;
        virtual void DispatchCompute(uint32 NumGroupsX, uint32 NumGroupsY, uint32 NumGroupsZ) override;
        virtual void DrawIndirect(RHIBuffer* Buffer, uint32 Offset) override;
        virtual void DrawIndexedIndirect(RHIBuffer* Buffer, uint32 Offset) override;
        virtual void DispatchComputeIndirect(RHIBuffer* Buffer, uint32 Offset) override;
        virtual void CopyBuffer(RHIBuffer* SrcBuffer, RHIBuffer* DstBuffer, uint32 SrcOffset, uint32 DstOffset, uint32 NumBytes) override;
        virtual void CopyBufferToImage(
            RHIBuffer* SrcBuffer, RHITexture* DstTexture, 
            int32 MipmapLevel, int32 Xoffset, int32 Yoffset, int32 Zoffset, 
            uint32 Width, uint32 Height, uint32 Depth, int32 BaseArrayLayer) override;
        virtual void BlitImage(RHITexture* SrcTexture, RHITexture* DstTexture) override NILOU_NOT_IMPLEMENTED;

        /* Set state commands */
        virtual void SetViewport(int32 Width, int32 Height) override NILOU_NOT_IMPLEMENTED;
        virtual void SetScissor(int32 Width, int32 Height) override NILOU_NOT_IMPLEMENTED;
        virtual void BindGraphicsPipelineState(RHIGraphicsPipelineState *NewPipelineState) override;
        virtual void BindComputePipelineState(RHIComputePipelineState *NewPipelineState) override;
        virtual void BindDescriptorSets(RHIPipelineLayout* PipelineLayout, const std::unordered_map<uint32, RHIDescriptorSet*>& DescriptorSets, EPipelineBindPoint PipelineBindPoint) override;
        virtual void BindIndexBuffer(RHIBuffer* Buffer, uint64 Offset) override;
        virtual void BindVertexBuffer(int32 BindingPoint, RHIBuffer* Buffer, uint64 Offset) override;
        virtual void PushConstants(RHIPipelineLayout* PipelineLayout, EShaderStage StageFlags, uint32 Offset, uint32 Size, const void* Data) override;

        /* Perform synchronization commands */
        virtual void PipelineBarrier(
            const std::vector<RHIMemoryBarrier>& MemoryBarriers, 
            const std::vector<RHIImageMemoryBarrier>& ImageMemoryBarriers, 
            const std::vector<RHIBufferMemoryBarrier>& BufferMemoryBarriers) override;

        enum class EState : uint8
        {
            ReadyForBegin,
            IsInsideBegin,
            IsInsideRenderPass,
            Submitted,
            NotAllocated,
        };
        EState State;

        void RefreshState();

        inline bool IsInsideRenderPass() const
        {
            return State == EState::IsInsideRenderPass;
        }
    
        inline bool IsOutsideRenderPass() const
        {
            return State == EState::IsInsideBegin;
        }

        inline bool HasBegun() const
        {
            return State == EState::IsInsideBegin || State == EState::IsInsideRenderPass;
        }

        inline bool IsSubmitted() const
        {
            return State == EState::Submitted;
        }

    private:

        VkDevice Device;
        VkCommandBuffer Handle;
        VkQueue Queue;
        VkCommandPool Pool;
        VkFence Fence;

        friend class FVulkanDynamicRHI;
        friend class VulkanCommandBufferPool;

    };

    inline VulkanCommandBuffer* ResourceCast(RHICommandList* RHICmdList)
    {
        return static_cast<VulkanCommandBuffer*>(RHICmdList);
    }
    
    class VulkanCommandBufferPool
    {
    public:

        VulkanCommandBufferPool(VkDevice InDevice, VkQueue InQueue, int32 QueueFamilyIndex);
        ~VulkanCommandBufferPool();

        VulkanCommandBuffer* Allocate();
        void FreeUnusedCmdBuffers();

        VkCommandPool Handle{};

        VkDevice Device;
        VkQueue Queue;

        std::vector<TRefCountPtr<VulkanCommandBuffer>> CmdBuffers;
        std::vector<TRefCountPtr<VulkanCommandBuffer>> FreeCmdBuffers;

    };

}