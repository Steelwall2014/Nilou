#include "Renderer/BasePassRendering.h"
#include "Renderer/RenderPass.h"
#include "Logging/LogMacros.h"
#include "Materials/Material.h"
#include "Renderer/DeferredShadingSceneRenderer.h"
#include "RHICommandList.h"
#include "DepthRendering.h"

namespace nilou {

    DEFINE_GRAPHICS_PIPELINE(FPreZPassPipeline, FBasePassVS, FDepthOnlyPS);

    void FDeferredShadingSceneRenderer::RenderPrePass(RenderGraph& Graph)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            std::vector<FMeshBatch>& MeshBatches = ViewMeshBatches[ViewIndex];
            FParallelMeshDrawCommands DrawCommands;

            RDGRenderTargets RenderTargets;
            RenderTargets.DepthStencilAttachment = { SceneTextures.DepthStencil->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };

            for (FMeshBatch &Mesh : MeshBatches)
            {
                for (FMeshBatchElement& Element : Mesh.Elements)
                {
                    FVertexFactoryPermutationParameters VertexFactoryParams(Element.VertexFactory->GetType(), Element.VertexFactory->GetPermutationId());
                    FGraphicsPipelinePermutationParameters PipelineParams(&FPreZPassPipeline::StaticType, 0);

                    FMeshDrawShaderBindings ShaderBindings = Mesh.MaterialRenderProxy->GetShaderBindings(Graph);
                    ShaderBindings.SetDescriptorSet("ViewParameters", View.ViewUniformBuffer->GetDescriptorSet());
                    ShaderBindings.SetDescriptorSet("PrimitiveParameters", Element.PrimitiveUniformBuffer->GetDescriptorSet());

                    RHIDepthStencilState* DepthStencilState = Mesh.MaterialRenderProxy->DepthStencilState.GetReference();
                    RHIRasterizerState* RasterizerState = Mesh.MaterialRenderProxy->RasterizerState.GetReference();
                    RHIBlendState* BlendState = Mesh.MaterialRenderProxy->BlendState.GetReference();
                    FMeshDrawCommand MeshDrawCommand;
                    BuildMeshDrawCommand(
                        Graph,
                        VertexFactoryParams,
                        Mesh.MaterialRenderProxy,
                        PipelineParams,
                        Element.VertexFactory->GetVertexDeclaration(),
                        Element,
                        RenderTargets.GetRenderTargetLayout(),
                        ShaderBindings,
                        DepthStencilState,
                        RasterizerState,
                        BlendState,
                        MeshDrawCommand);

                    DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                }
            }

            RDGPassDesc PassDesc{NFormat("PreZPass {}", ViewIndex)};
            PassDesc.bNeverCull = true;
            Graph.AddGraphicsPass(
                PassDesc,
                RenderTargets,
                DrawCommands.GetIndexBuffers(),
                DrawCommands.GetVertexBuffers(),
                [&DrawCommands](FRDGPass* Pass)
                {
                    for (RDGDescriptorSet* DS : DrawCommands.GetDescriptorSets())
                        Pass->DescriptorSets.push_back(DS);
                },
                [DrawCommands](RHICommandList& RHICmdList)
                {
                    DrawCommands.DispatchDraw(RHICmdList);
                }
            );

        }
    }

}