#pragma once

#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RHICommandContext.h"

namespace nilou {

    class RHIMemoryBarrier
    {
    };

    class RHIImageMemoryBarrier
    {
    };

    class RHIBufferMemoryBarrier
    {
    };

    enum class EPipelineBindPoint
    {
        Graphics = 0,
        Compute,
        RayTracing,

        Num
    };

    enum class EImmediateFlushType
    {
		WaitForOutstandingTasksOnly  = 0, 
		DispatchToRHIThread          = 1, 
		FlushRHIThread               = 2,
		FlushRHIThreadFlushResources = 3
    };

    class RHICommandList
    {
    public:

        /* Perform actions commands */
        virtual void BeginRenderPass(FRHIRenderPassInfo& Info) = 0;
        virtual void EndRenderPass() = 0;
        virtual void DrawArrays(uint32 VertexCount, uint32 InstanceCount, uint32 FirstVertex, uint32 FirstInstance) = 0;
        virtual void DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, uint32 VertexOffset, uint32 FirstInstance) = 0;
        virtual void DispatchCompute(uint32 NumGroupsX, uint32 NumGroupsY, uint32 NumGroupsZ) = 0;
        virtual void DrawIndirect(RHIBuffer* Buffer, uint32 Offset) = 0;
        virtual void DrawIndexedIndirect(RHIBuffer* Buffer, uint32 Offset) = 0;
        virtual void DispatchComputeIndirect(RHIBuffer* Buffer, uint32 Offset) = 0;
        virtual void CopyBuffer(RHIBuffer* SrcBuffer, RHIBuffer* DstBuffer, uint32 SrcOffset, uint32 DstOffset, uint32 NumBytes) = 0;
        virtual void CopyBufferToImage(
            RHIBuffer* SrcBuffer, RHITexture* DstTexture, 
            int32 MipmapLevel, int32 Xoffset, int32 Yoffset, int32 Zoffset, 
            uint32 Width, uint32 Height, uint32 Depth, int32 BaseArrayLayer) = 0;
        virtual void CopyImageToBuffer(
            RHITexture* SrcTexture, RHIBuffer* DstBuffer, 
            int32 MipmapLevel, int32 Xoffset, int32 Yoffset, int32 Zoffset, 
            uint32 Width, uint32 Height, uint32 Depth, int32 BaseArrayLayer) = 0;
        virtual void BlitImage(RHITexture* SrcTexture, RHITexture* DstTexture) = 0;

        /* Set state commands */
        virtual void SetViewport(int32 Width, int32 Height) = 0;
        virtual void SetScissor(int32 Width, int32 Height) = 0;
        virtual void BindPipeline(FRHIPipelineState *NewPipelineState, EPipelineBindPoint PipelineBindPoint) = 0;
        // TODO: 测试一下到底用map还是unordered_map
        virtual void BindDescriptorSets(RHIPipelineLayout* PipelineLayout, const std::unordered_map<uint32, RHIDescriptorSet*>& DescriptorSets, EPipelineBindPoint PipelineBindPoint) = 0;
        virtual void BindIndexBuffer(RHIBuffer* Buffer, uint64 Offset) = 0;
        virtual void BindVertexBuffer(int32 BindingPoint, RHIBuffer* Buffer, uint64 Offset) = 0;
        virtual void PushConstants(RHIPipelineLayout* PipelineLayout, EPipelineBindPoint PipelineBindPoint, uint32 Offset, uint32 Size, const void* Data) = 0;

        /* Perform synchronization commands */
        virtual void PipelineBarrier(
            const std::vector<RHIMemoryBarrier>& MemoryBarriers, 
            const std::vector<RHIImageMemoryBarrier>& ImageMemoryBarriers, 
            const std::vector<RHIBufferMemoryBarrier>& BufferMemoryBarriers) = 0;

        RHICommandContext& GetContext() 
        {
            Ncheck(GraphicsContext);
            return *GraphicsContext; 
        }
        RHICommandContext& GetComputeContext() 
        { 
            Ncheck(ComputeContext);
            return *ComputeContext; 
        }

    protected:

        RHICommandContext* GraphicsContext;
        RHICommandContext* ComputeContext;

    };

    class RHICommandListImmediate : public RHICommandList
    {
    public:
        //
        // Executes commands recorded in the immediate RHI command list, and resets the command list to a default constructed state.
        //
        // This is the main function for submitting work from the render thread to the RHI thread. Work is also submitted to the GPU
        // as soon as possible. Does not wait for command completion on either the RHI thread or the GPU.
        //
        // Used internally. Do not call directly. Use FRHICommandListImmediate::ImmediateFlush() to submit GPU work.
        //
        void ExecuteAndReset();
        
        //
        // Dispatches work to the RHI thread and the GPU.
        // Also optionally waits for its completion on the RHI thread. Does not wait for the GPU.
        //
        virtual void ImmediateFlush(EImmediateFlushType FlushType);
    };

}