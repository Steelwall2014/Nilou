#include "BasePassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DeferredShadingSceneRenderer.h"
#include "RHICommandList.h"


namespace nilou {
    IMPLEMENT_SHADER_TYPE(FBasePassVS, "/Shaders/MaterialShaders/BasePassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FBasePassPS, "/Shaders/MaterialShaders/BasePassPixelShader.frag", EShaderFrequency::SF_Pixel, Material);

    void FDeferredShadingSceneRenderer::RenderBasePass(RenderGraph& Graph)
    {    
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            std::vector<FMeshBatch>& MeshBatches = ViewMeshBatches[ViewIndex];
            FParallelMeshDrawCommands DrawCommands;

            RDGRenderTargets RenderTargets;
            RenderTargets.ColorAttachments[0] = SceneTextures.BaseColor->GetDefaultView();
            RenderTargets.ColorAttachments[1] = SceneTextures.RelativeWorldSpacePosition->GetDefaultView();
            RenderTargets.ColorAttachments[2] = SceneTextures.WorldSpaceNormal->GetDefaultView();
            RenderTargets.ColorAttachments[3] = SceneTextures.MetallicRoughness->GetDefaultView();
            RenderTargets.ColorAttachments[4] = SceneTextures.Emissive->GetDefaultView();
            RenderTargets.ColorAttachments[5] = SceneTextures.ShadingModel->GetDefaultView();
            RenderTargets.DepthStencilAttachment = SceneTextures.DepthStencil->GetDefaultView();

            std::vector<RDGDescriptorSet*> DescriptorSets;
            RDGDescriptorSet* DescriptorSet_VS = Graph.CreateDescriptorSet<FBasePassVS>(0, VERTEX_SHADER_SET_INDEX);
            DescriptorSet_VS->SetUniformBuffer("FViewShaderParameters", View.ViewUniformBuffer);
            DescriptorSets.push_back(DescriptorSet_VS);

            for (FMeshBatch &Mesh : MeshBatches)
            {
                for (auto& [SetIndex, DescriptorSet] : Mesh.MaterialRenderProxy->DescriptorSets)
                {
                    DescriptorSets.push_back(DescriptorSet);
                }
                for (FMeshBatchElement& Element : Mesh.Elements)
                {
                    FVertexFactoryPermutationParameters VertexFactoryParams(Element.VertexFactory->GetType(), Element.VertexFactory->GetPermutationId());
                    FShaderPermutationParameters PermutationParametersVS(&FBasePassVS::StaticType, 0);
                    FShaderPermutationParameters PermutationParametersPS(&FBasePassPS::StaticType, 0);

                    FMeshDrawCommand MeshDrawCommand;
                    MeshDrawCommand.ShaderBindings.SetDescriptorSet(VERTEX_SHADER_SET_INDEX, DescriptorSet_VS);

                    BuildMeshDrawCommand(
                        VertexFactoryParams,
                        Mesh.MaterialRenderProxy,
                        PermutationParametersVS,
                        PermutationParametersPS,
                        Element.VertexFactory->GetVertexDeclaration(),
                        Element,
                        RenderTargets.GetRenderTargetLayout(),
                        MeshDrawCommand);

                    DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                    RHIGetError();
                }
                
            }

            RDGPassDesc PassDesc{NFormat("BasePass {}", ViewIndex)};
            PassDesc.bNeverCull = true;
            Graph.AddGraphicsPass(
                PassDesc,
                RenderTargets,
                {},
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