#include "PreZPassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DeferredShadingSceneRenderer.h"
#include "RHICommandList.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FPreZPassVS, "/Shaders/MaterialShaders/PreZPassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FPreZPassPS, "/Shaders/MaterialShaders/DepthOnlyPixelShader.frag", EShaderFrequency::SF_Pixel, Material);

    void FDeferredShadingSceneRenderer::RenderPreZPass(RenderGraph& Graph)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            std::vector<FMeshBatch>& MeshBatches = ViewMeshBatches[ViewIndex];
            FParallelMeshDrawCommands DrawCommands;

            RDGRenderTargets RenderTargets;
            RenderTargets.DepthStencilAttachment = SceneTextures.DepthStencil->GetDefaultView();

            RDGDescriptorSet* DescriptorSet_VS = Graph.CreateDescriptorSet<FPreZPassVS>(0, VERTEX_SHADER_SET_INDEX);
            DescriptorSet_VS->SetUniformBuffer("FViewShaderParameters", View.ViewUniformBuffer);

            for (FMeshBatch &Mesh : MeshBatches)
            {
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
                        RenderTargets.GetRenderTargetLayout(),
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