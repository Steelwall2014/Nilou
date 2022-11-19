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
            RHICmdList->GLDEBUG();
        RHICmdList->RHISetGraphicsPipelineState(PipelineState);
        RHICmdList->RHISetDepthStencilState(DepthStencilState.get());
        RHICmdList->RHISetRasterizerState(RasterizerState.get());
        RHICmdList->RHISetBlendState(BlendState.get());
            RHICmdList->GLDEBUG();
        FRHIGraphicsPipelineState *PSO = PipelineState;
            RHICmdList->GLDEBUG();
        for (int PipelineStage = 0; PipelineStage < EPipelineStage::PipelineStageNum; PipelineStage++)
        {
            for (auto [BindingPoint, UniformBufferRHI] : ShaderBindings.UniformBufferBindings[PipelineStage])
            {
                RHICmdList->RHISetShaderUniformBuffer(PSO, (EPipelineStage)PipelineStage, BindingPoint, UniformBufferRHI);
            RHICmdList->GLDEBUG();
            }
            for (auto [BindingPoint, Sampler] : ShaderBindings.SamplerBindings[PipelineStage])
            {
                RHICmdList->RHISetShaderSampler(PSO, (EPipelineStage)PipelineStage, BindingPoint, *Sampler);
            RHICmdList->GLDEBUG();
            }
        }

        for (FRHIVertexInput &VertexInput : *ShaderBindings.VertexAttributeBindings)
        {
            RHICmdList->RHISetVertexBuffer(PSO, &VertexInput);
            RHICmdList->GLDEBUG();
        }
        if (UseIndirect)
        {
            RHICmdList->RHIDrawIndexedIndirect(IndexBuffer, IndirectArgs.Buffer, IndirectArgs.Offset);
            RHICmdList->GLDEBUG();
        }
        else 
        {
            if (IndexBuffer)
                RHICmdList->RHIDrawIndexed(IndexBuffer, DirectArgs.NumInstances);
            else 
                RHICmdList->RHIDrawArrays(DirectArgs.NumVertices, DirectArgs.NumInstances);
            RHICmdList->GLDEBUG();
        }
    }

}