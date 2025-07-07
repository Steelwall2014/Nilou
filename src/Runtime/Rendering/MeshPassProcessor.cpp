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
    {
        std::memset(&IndirectArgs, 0, sizeof(IndirectArgs));
    }

    void FMeshDrawCommand::SubmitDraw(RHICommandList& RHICmdList) const
    {
        RHIGetError();
        RHICmdList.BindGraphicsPipelineState(PipelineState);

        for (auto& Stream : VertexStreams)
        {
            RHICmdList.BindVertexBuffer(Stream.StreamIndex, Stream.VertexBuffer->GetRHI(), Stream.Offset);
        }
        RHIGetError();

        if (DescriptorSets.size() > 0)
        {
            std::unordered_map<uint32, RHIDescriptorSet*> DescriptorSetsRHI;
            for (auto& [SetIndex, DescriptorSet] : DescriptorSets)
            {
                DescriptorSetsRHI[SetIndex] = DescriptorSet->GetRHI();
            }
            RHICmdList.BindDescriptorSets(PipelineState->GetPipelineLayout(), DescriptorSetsRHI, EPipelineBindPoint::Graphics);
        }

        for (auto& [Stage, PushConstants] : PushConstants)
        {
            RHICmdList.PushConstants(PipelineState->GetPipelineLayout(), Stage, 0, PushConstants.size(), PushConstants.data());
        }

        if (bUseIndirectArgs)
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