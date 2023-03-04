#include "MeshPassProcessor.h"
#include "DynamicRHI.h"
#include "Material.h"
#include "RHI.h"
#include "RHIResources.h"
#include "Shader.h"
#include "ShaderMap.h"
#include <set>

namespace nilou {

    FMeshDrawCommand::FMeshDrawCommand()
        : IndexBuffer(nullptr)
        , PipelineState(nullptr)
        , StencilRef(0)
        , DepthStencilState(nullptr)
        , RasterizerState(nullptr)
        , BlendState(nullptr)
        , UseIndirect(false)
    {

    }

    void FMeshDrawCommand::SubmitDraw(FDynamicRHI *RHICmdList)
    {
        RHIGetError();
        RHICmdList->RHISetGraphicsPipelineState(PipelineState);
        RHICmdList->RHISetDepthStencilState(DepthStencilState.get(), StencilRef);
        RHICmdList->RHISetRasterizerState(RasterizerState.get());
        RHICmdList->RHISetBlendState(BlendState.get());
        RHIGetError();
        FRHIGraphicsPipelineState *PSO = PipelineState;
        RHIGetError();
        for (int PipelineStage = 0; PipelineStage < EPipelineStage::PipelineStageNum; PipelineStage++)
        {
            for (auto &[BindingPoint, UniformBufferRHI] : ShaderBindings.UniformBufferBindings[PipelineStage])
            {
                RHICmdList->RHISetShaderUniformBuffer(PSO, (EPipelineStage)PipelineStage, BindingPoint, UniformBufferRHI);
                RHIGetError();
            }
            for (auto &[BindingPoint, Sampler] : ShaderBindings.SamplerBindings[PipelineStage])
            {
                RHICmdList->RHISetShaderSampler(PSO, (EPipelineStage)PipelineStage, BindingPoint, *Sampler);
                RHIGetError();
            }
            for (auto &[BindingPoint, Buffer] : ShaderBindings.BufferBindings[PipelineStage])
            {
                RHICmdList->RHIBindComputeBuffer(PSO, (EPipelineStage)PipelineStage, BindingPoint, Buffer);
                RHIGetError();
            }
        }

        for (FRHIVertexInput &VertexInput : ShaderBindings.VertexAttributeBindings)
        {
            RHICmdList->RHISetVertexBuffer(PSO, &VertexInput);
            RHIGetError();
        }
        if (UseIndirect)
        {
            RHICmdList->RHIDrawIndexedIndirect(IndexBuffer, IndirectArgs.Buffer, IndirectArgs.Offset);
            RHIGetError();
        }
        else 
        {
            if (IndexBuffer)
                RHICmdList->RHIDrawIndexed(IndexBuffer, DirectArgs.NumInstances);
            else 
                RHICmdList->RHIDrawArrays(DirectArgs.BaseVertexIndex, DirectArgs.NumVertices, DirectArgs.NumInstances);
            RHIGetError();
        }
    }

}