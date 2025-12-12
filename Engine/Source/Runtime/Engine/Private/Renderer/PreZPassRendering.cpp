#include "Renderer/BasePassRendering.h"
#include "Renderer/RenderPass.h"
#include "Logging/LogMacros.h"
#include "Materials/Material.h"
#include "Renderer/DeferredShadingSceneRenderer.h"
#include "RHICommandList.h"
#include "DepthRendering.h"

namespace nilou {

    void FDeferredShadingSceneRenderer::RenderPrePass(RenderGraph& Graph)
    {
        RHIDepthStencilState* DepthStencilState = TStaticDepthStencilState<true, CF_LessEqual>::GetRHI();
        RHIRasterizerState* RasterizerState = TStaticRasterizerState<FM_Solid, CM_CW>::GetRHI();
        RHIBlendState* BlendState = TStaticBlendState<>::GetRHI();
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
                    FShaderPermutationParameters PermutationParametersVS(&FBasePassVS::StaticType, 0);
                    FShaderPermutationParameters PermutationParametersPS(&FDepthOnlyPS::StaticType, 0);

                    FMeshDrawShaderBindings ShaderBindings = Mesh.MaterialRenderProxy->GetShaderBindings();
                    ShaderBindings.SetBuffer("ViewParameters", View.ViewUniformBuffer);
                    ShaderBindings.SetBuffer("PrimitiveParameters", Element.PrimitiveUniformBuffer);

                    FMeshDrawCommand MeshDrawCommand;
                    BuildMeshDrawCommand(
                        Graph,
                        VertexFactoryParams,
                        Mesh.MaterialRenderProxy,
                        PermutationParametersVS,
                        PermutationParametersPS,
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
                DrawCommands.GetDescriptorSets(),
                [=](RHICommandList& RHICmdList)
                {
                    DrawCommands.DispatchDraw(RHICmdList);
                }
            );

        }
    }

}