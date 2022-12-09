#include "MeshPassProcessor.h"
#include "DynamicRHI.h"
#include "Material.h"
#include "RHI.h"
#include "RHIResources.h"
#include "Shader.h"
#include "ShaderMap.h"
#include <set>

namespace nilou {

    // void FBasePassMeshProcessor::AddMeshBatch(const FMeshBatch &Mesh, std::vector<FMeshDrawCommand> *OutDrawCommandList)
    // {

        
    //     // FBasePassVS *BasePassVS = FBasePassVS::Shader;
    //     // std::set<std::string> VertexFactoryDefinitions = Mesh.VertexFactory->CalcDefinitions();
    //     // StateData.VertexShader = BasePassVS->GetOrCreateShaderByPermutation(Mesh.VertexFactory, VertexFactoryDefinitions);
    //     // StateData.VertexShader = Mesh.VertexFactory->GetShader();

        
    // }

    void FMeshDrawCommand::SubmitDraw(FDynamicRHI *RHICmdList)
    {
        RHIGetError();
        RHICmdList->RHISetGraphicsPipelineState(PipelineState);
        RHICmdList->RHISetDepthStencilState(DepthStencilState.get());
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
        }

        for (FRHIVertexInput &VertexInput : *ShaderBindings.VertexAttributeBindings)
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
                RHICmdList->RHIDrawArrays(DirectArgs.NumVertices, DirectArgs.NumInstances);
            RHIGetError();
        }
    }

}