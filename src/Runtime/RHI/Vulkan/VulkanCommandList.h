#include "RHICommandList.h"

namespace nilou {
    class FVulkanCmdBuffer;
    class FVulkanQueue;

    class VulkanCommandList : public RHICommandList
    {
    public:

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
        virtual void BlitImage(RHITexture* SrcTexture, RHITexture* DstTexture) override;

        /* Set state commands */
        virtual void SetViewport(int32 Width, int32 Height) override;
        virtual void SetScissor(int32 Width, int32 Height) override;
        virtual void BindPipeline(FRHIPipelineState *NewPipelineState, EPipelineBindPoint PipelineBindPoint) override;
        virtual void BindDescriptorSets(RHIPipelineLayout* PipelineLayout, const std::unordered_map<uint32, RHIDescriptorSet*>& DescriptorSets, EPipelineBindPoint PipelineBindPoint) override;
        virtual void BindIndexBuffer(RHIBuffer* Buffer, uint64 Offset) override;
        virtual void BindVertexBuffer(int32 BindingPoint, RHIBuffer* Buffer, uint64 Offset) override;
        virtual void PushConstants(RHIPipelineLayout* PipelineLayout, EPipelineBindPoint PipelineBindPoint, uint32 Offset, uint32 Size, const void* Data) override;

        /* Perform synchronization commands */
        virtual void PipelineBarrier(
            const std::vector<RHIMemoryBarrier>& MemoryBarriers, 
            const std::vector<RHIImageMemoryBarrier>& ImageMemoryBarriers, 
            const std::vector<RHIBufferMemoryBarrier>& BufferMemoryBarriers) override;

        virtual void Submit(const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal) = 0;

    private:

        FVulkanCmdBuffer* CmdBuffer;
        FVulkanQueue* Queue;

    };

}