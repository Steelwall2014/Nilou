#include <set>
#include "MeshPassProcessor.h"
#include "DynamicRHI.h"
#include "Material.h"
#include "RHI.h"
#include "RHIResources.h"
#include "Shader.h"
#include "ShaderMap.h"
#include "RHICommandList.h"

namespace nilou {

    FMeshDrawCommand::FMeshDrawCommand()
        : IndexBuffer(nullptr)
        , PipelineState(nullptr)
        , StencilRef(0)
    {

    }

    void FMeshDrawCommand::SubmitDraw(RHICommandList& RHICmdList) const
    {
        RHIGetError();
        RHICmdList.BindPipeline(PipelineState, EPipelineBindPoint::Graphics);

        for (auto& Stream : VertexStreams)
        {
            RHICmdList.BindVertexBuffer(Stream.StreamIndex, Stream.VertexBuffer->GetRHI(), Stream.Offset);
        }
        RHIGetError();

        std::unordered_map<uint32, RHIDescriptorSet*> DescriptorSetsRHI;
        for (auto& [SetIndex, DescriptorSet] : ShaderBindings.DescriptorSets)
        {
            DescriptorSetsRHI[SetIndex] = DescriptorSet->GetRHI();
        }
        RHICmdList.BindDescriptorSets(PipelineState->PipelineLayout.get(), DescriptorSetsRHI, EPipelineBindPoint::Graphics);

        if (IndirectArgs.Buffer)
        {
            RHICmdList.BindIndexBuffer(IndexBuffer->GetRHI(), 0);
            RHICmdList.DrawIndexedIndirect(IndirectArgs.Buffer, IndirectArgs.Offset);
            RHIGetError();
        }
        else 
        {
            if (IndexBuffer)
            {
                RHICmdList.BindIndexBuffer(IndexBuffer->GetRHI(), 0);
                RHICmdList.DrawIndexed(IndexBuffer->GetRHI()->GetCount(), NumInstances, 0, 0, 0);
            }
            else 
            {
                RHICmdList.DrawArrays(VertexParams.NumVertices, NumInstances, VertexParams.BaseVertexIndex, 0);
            }
            RHIGetError();
        }
    }

}