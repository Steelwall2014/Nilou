#include "PreZPassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DeferredShadingSceneRenderer.h"
#include "RHICommandList.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FPreZPassVS, "/Shaders/MaterialShaders/PreZPassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FPreZPassPS, "/Shaders/GlobalShaders/DepthOnlyPixelShader.frag", EShaderFrequency::SF_Pixel, Global);

    void FDeferredShadingSceneRenderer::RenderPreZPass(RenderGraph& Graph)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            std::vector<FMeshBatch>& MeshBatches = ViewMeshBatches[ViewIndex];
            FParallelMeshDrawCommands DrawCommands;

            RDGFramebuffer Framebuffer;
            Framebuffer.SetAttachment(FA_Depth_Stencil_Attachment, SceneTextures.DepthStencil->GetDefaultView());
            const RHIRenderTargetLayout& RTLayout = Framebuffer.GetRenderTargetLayout();

            std::vector<RDGDescriptorSet*> DescriptorSets;
            RDGDescriptorSet* DescriptorSet_VS = Graph.CreateDescriptorSet<FPreZPassVS>(0, VERTEX_SHADER_SET_INDEX);
            DescriptorSet_VS->SetUniformBuffer("FViewShaderParameters", View.ViewUniformBuffer);
            DescriptorSets.push_back(DescriptorSet_VS);

            for (FMeshBatch &Mesh : MeshBatches)
            {
                for (auto& [SetIndex, DescriptorSet] : Mesh.MaterialRenderProxy->DescriptorSets)
                {
                    DescriptorSets.push_back(DescriptorSet.get());
                }
                for (FMeshBatchElement& Element : Mesh.Elements)
                {
                    FVertexFactoryPermutationParameters VertexFactoryParams(Element.VertexFactory->GetType(), Element.VertexFactory->GetPermutationId());
                    FShaderPermutationParameters PermutationParametersVS(&FPreZPassVS::StaticType, 0);
                    FShaderPermutationParameters PermutationParametersPS(&FPreZPassPS::StaticType, 0);

                    FMeshDrawCommand MeshDrawCommand;
                    MeshDrawCommand.ShaderBindings.SetDescriptorSet(VERTEX_SHADER_SET_INDEX, DescriptorSet_VS);
                    
                    BuildMeshDrawCommand(
                        VertexFactoryParams,
                        Mesh.MaterialRenderProxy,
                        PermutationParametersVS,
                        PermutationParametersPS,
                        Element.VertexFactory->GetVertexDeclaration(),
                        Element,
                        RTLayout,
                        MeshDrawCommand);

                    DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                }
            }

            RDGGraphicsPassDesc PassDesc;
            PassDesc.Name = "PreZPass";
            PassDesc.RenderTargets = Framebuffer;
            PassDesc.DescriptorSets = DescriptorSets;
            Graph.AddGraphicsPass(
                PassDesc,
                [=](RHICommandList& RHICmdList)
                {
                    DrawCommands.DispatchDraw(RHICmdList);
                }
            );

        }
    }

}