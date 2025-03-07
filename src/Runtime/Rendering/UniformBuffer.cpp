#include <memory>
#include "DynamicRHI.h"
#include "RHIDefinitions.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

namespace nilou {

    void FUniformBuffer::InitRHI_impl(RenderGraph& Graph)
    {
        RDGBufferDesc Desc;
        Desc.BytesPerElement = Size;
        Desc.NumElements = 1;
        if (Usage == UniformBuffer_MultiFrame)
        {
            UniformBufferRDG = RenderGraph::CreateExternalBuffer("", Desc);
        }
        else 
        {
            UniformBufferRDG = Graph.CreateBuffer("", Desc);
        }
    }

    void FUniformBuffer::UploadData_impl(RenderGraph& Graph, const void* Data, uint32 DataSize)
    {
        // DataSize = std::min(DataSize, Size);
        // StagingBuffer* StagingBuffer = Graph.AcquireStagingBuffer(DataSize);
        // Graph.AddPass(
        //     [=](RDGPassBuilder& PassBuilder)
        //     {
        //         FDynamicRHI* DynamicRHI = FDynamicRHI::GetDynamicRHI();
        //         void* Dst = DynamicRHI->MapMemory(StagingBuffer->BufferRHI.get(), 0, DataSize);
        //             std::memcpy(Dst, Data, DataSize);
        //         DynamicRHI->UnmapMemory(StagingBuffer->BufferRHI.get());

        //         PassBuilder.TransferDestination(UniformBufferRDG);
        //     },
        //     [=](RenderGraph& Graph)
        //     {
        //         RHIBuffer* SrcBuffer = StagingBuffer->BufferRHI.get();
        //         RHIBuffer* DstBuffer = UniformBufferRDG->Resolve();
        //         if (SrcBuffer && DstBuffer)
        //         {
        //             RHICmdList.CopyBuffer(SrcBuffer, DstBuffer, 0, 0, DataSize);
        //         }
        //     });
    }

}