#include "BasePassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DeferredShadingSceneRenderer.h"
#include "RHICommandList.h"
#include "RenderGraphUtils.h"


namespace nilou {
    IMPLEMENT_SHADER_TYPE(FBasePassVS, "/Shaders/MaterialShaders/BasePassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FBasePassPS, "/Shaders/MaterialShaders/BasePassPixelShader.frag", EShaderFrequency::SF_Pixel, Material);

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
            RenderTargets.ColorAttachments[0] = { SceneTextures.BaseColor->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[1] = { SceneTextures.RelativeWorldSpacePosition->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[2] = { SceneTextures.WorldSpaceNormal->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[3] = { SceneTextures.MetallicRoughness->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[4] = { SceneTextures.Emissive->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.ColorAttachments[5] = { SceneTextures.ShadingModel->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.DepthStencilAttachment = { SceneTextures.DepthStencil->GetDefaultView(), ERenderTargetLoadAction::Load, ERenderTargetStoreAction::Store };

            for (FMeshBatch &Mesh : MeshBatches)
            {
                for (FMeshBatchElement& Element : Mesh.Elements)
                {
                    FVertexFactoryPermutationParameters VertexFactoryParams(Element.VertexFactory->GetType(), Element.VertexFactory->GetPermutationId());
                    FShaderPermutationParameters PermutationParametersVS(&FBasePassVS::StaticType, 0);
                    FShaderPermutationParameters PermutationParametersPS(&FBasePassPS::StaticType, 0);

                    FMeshDrawShaderBindings ShaderBindings = Mesh.MaterialRenderProxy->GetShaderBindings();
                    ShaderBindings.SetBuffer("FViewShaderParameters", View.ViewUniformBuffer);
                    ShaderBindings.SetBuffer("FPrimitiveShaderParameters", Element.PrimitiveUniformBuffer);

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
                DrawCommands.GetDescriptorSets(),
                [=](RHICommandList& RHICmdList)
                {
                    // FRHIRenderPassInfo PassInfo(SceneTextures->GeometryPassFramebuffer.get(), ViewInfo.ScreenResolution, true);
                    // RHICmdList.BeginRenderPass(PassInfo);
                    DrawCommands.DispatchDraw(RHICmdList);
                    // RHICmdList.EndRenderPass();
                }
            );
        }
    


    }
}