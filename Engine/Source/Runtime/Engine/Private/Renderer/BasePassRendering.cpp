#include "Renderer/BasePassRendering.h"
#include "Logging/LogMacros.h"
#include "Materials/Material.h"
#include "Renderer/DeferredShadingSceneRenderer.h"
#include "Renderer/RenderPass.h"
#include "RHICommandList.h"
#include "RenderGraphUtils.h"


namespace nilou {
    IMPLEMENT_SHADER_TYPE(FBasePassVS, "/Shaders/Private/MaterialShaders/BasePassVertexShader.slang", "MainVS", EShaderFrequency::Vertex);
    IMPLEMENT_SHADER_TYPE(FBasePassPS, "/Shaders/Private/MaterialShaders/BasePassPixelShader.slang", "MainPS", EShaderFrequency::Pixel);
    IMPLEMENT_GRAPHICS_PIPELINE(FBasePassPipeline)

    void FDeferredShadingSceneRenderer::RenderBasePass(RenderGraph& Graph)
    {    
        RHIDepthStencilState* DepthStencilState = TStaticDepthStencilState<true, CF_LessEqual>::GetRHI();
        RHIBlendState* BlendState = TStaticBlendState<>::GetRHI();
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            std::vector<FMeshBatch>& MeshBatches = ViewMeshBatches[ViewIndex];
            FParallelMeshDrawCommands DrawCommands;

            RDGRenderTargets RenderTargets;
            RenderTargets.ColorAttachments[0] = { SceneTextures.SceneColor->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[1] = { SceneTextures.BaseColor->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[2] = { SceneTextures.RelativeWorldSpacePosition->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[3] = { SceneTextures.WorldSpaceNormal->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[4] = { SceneTextures.MetallicRoughness->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[5] = { SceneTextures.Emissive->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[6] = { SceneTextures.ShadingModel->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.DepthStencilAttachment = { SceneTextures.DepthStencil->GetDefaultView(), ERenderTargetLoadAction::Load, ERenderTargetStoreAction::Store };

            for (FMeshBatch &Mesh : MeshBatches)
            {
                for (FMeshBatchElement& Element : Mesh.Elements)
                {
                    FVertexFactoryPermutationParameters VertexFactoryPermutation(Element.VertexFactory->GetType(), Element.VertexFactory->GetPermutationId());
                    FBasePassPipeline::FPermutationDomain Domain;
                    Domain.Set<FBasePassPipeline::FDimensionEnableIBL>(false);
                    FGraphicsPipelinePermutationParameters PipelinePermutation(&FBasePassPipeline::StaticType, Domain.ToDimensionValueId());

                    FMeshDrawShaderBindings ShaderBindings = Mesh.MaterialRenderProxy->GetShaderBindings(Graph);
                    ShaderBindings.SetDescriptorSet("ViewParameters", View.ViewUniformBuffer->GetDescriptorSet());
                    ShaderBindings.SetDescriptorSet("PrimitiveParameters", Element.PrimitiveUniformBuffer->GetDescriptorSet());

                    FMeshDrawCommand MeshDrawCommand;
                    BuildMeshDrawCommand(
                        Graph,
                        VertexFactoryPermutation,
                        Mesh.MaterialRenderProxy,
                        PipelinePermutation,
                        Element.VertexFactory->GetVertexDeclaration(),
                        Element,
                        RenderTargets.GetRenderTargetLayout(),
                        ShaderBindings,
                        DepthStencilState,
                        Mesh.MaterialRenderProxy->RasterizerState.GetReference(),
                        BlendState,
                        MeshDrawCommand);

                    DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                }
                
            }

            RDGPassDesc PassDesc{NFormat("BasePass {}", ViewIndex)};
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