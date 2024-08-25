#include "PreZPassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DefferedShadingSceneRenderer.h"
#include "RHICommandList.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FPreZPassVS, "/Shaders/MaterialShaders/PreZPassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FPreZPassPS, "/Shaders/GlobalShaders/DepthOnlyPixelShader.frag", EShaderFrequency::SF_Pixel, Global);


    static void BuildMeshDrawCommand(
        const FVertexFactoryPermutationParameters &VFPermutationParameters,
        FMaterialRenderProxy *MaterialProxy,
        const FShaderPermutationParameters &PermutationParametersVS,
        const FShaderPermutationParameters &PermutationParametersPS,
        FInputShaderBindings &InputBindings,  
        FRHIVertexDeclaration* VertexDeclaration,
        const FMeshBatchElement &Element,
        const RenderTargetLayout &RTLayout,
        FMeshDrawCommand &OutMeshDrawCommand
    )
    {
        
        FGraphicsPipelineStateInitializer Initializer;
        FShaderInstance *VertexShader = MaterialProxy->GetShader(VFPermutationParameters, PermutationParametersVS);
        Initializer.VertexShader = VertexShader->GetVertexShaderRHI();
        FShaderInstance *PixelShader = GetGlobalShader(PermutationParametersPS);
        Initializer.PixelShader = PixelShader->GetPixelShaderRHI();
        Initializer.DepthStencilState = MaterialProxy->DepthStencilState.get();
        Initializer.RasterizerState = MaterialProxy->RasterizerState.get();
        Initializer.BlendState = MaterialProxy->BlendState.get();
        Initializer.VertexDeclaration = VertexDeclaration;
        Initializer.RTLayout = RTLayout;

        {
            OutMeshDrawCommand.StencilRef = MaterialProxy->StencilRefValue;
            OutMeshDrawCommand.VertexStreams = Element.VertexFactory->GetVertexInputStreams();
            OutMeshDrawCommand.PipelineState = RHICreateGraphicsPipelineState(Initializer);
            RHIGetError();
            OutMeshDrawCommand.IndexBuffer = Element.IndexBuffer->IndexBufferRDG->Resolve();

            MaterialProxy->FillShaderBindings(InputBindings);
       
            FRHIDescriptorSet &DescriptorSets = OutMeshDrawCommand.PipelineState->PipelineLayout->DescriptorSets[PS_Vertex];
            
            for (auto [Name,Binding] : DescriptorSets.Bindings)
            {
                bool bResourceFound = OutMeshDrawCommand.ShaderBindings.SetShaderBinding(
                    static_cast<EPipelineStage>(PS_Vertex), Binding, InputBindings);

                if (!bResourceFound)
                {
                        NILOU_LOG(Warning, 
                            "Material: {}"
                            " |Vertex Factory: {}"
                            " |Vertex Shader: {}"
                            " |Pixel Shader: {}"
                            " |Pipeline Stage: {}"
                            " |\"{}\" Resource not provided",
                            MaterialProxy->Material->Name,
                            VFPermutationParameters.Type->Name,
                            PermutationParametersVS.Type->Name,
                            PermutationParametersVS.Type->Name,
                            magic_enum::enum_name(PS_Vertex),
                            Binding.Name);
                }

            }

            if (Element.NumVertices == 0)
            {
                OutMeshDrawCommand.IndirectArgs.Buffer = Element.IndirectArgsBuffer;
                OutMeshDrawCommand.IndirectArgs.Offset = Element.IndirectArgsOffset;
                OutMeshDrawCommand.UseIndirect = true;
            }
            else
            {
                OutMeshDrawCommand.DirectArgs.BaseVertexIndex = Element.FirstIndex;
                OutMeshDrawCommand.DirectArgs.NumInstances = Element.NumInstances;
                OutMeshDrawCommand.DirectArgs.NumVertices = Element.NumVertices;
                OutMeshDrawCommand.UseIndirect = false;
            }
        }
    }

    void FDefferedShadingSceneRenderer::RenderPreZPass(RenderGraph& Graph)
    {

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            FParallelMeshDrawCommands DrawCommands;

            // Build the render target layout of pre-z pass
            RenderTargetLayout RTLayout;
            RTLayout.NumRenderTargetsEnabled = 1;
            RTLayout.DepthStencilTargetFormat = SceneTextures.DepthStencil->Desc.Format;

            for (FMeshBatch &Mesh : Views[ViewIndex].DynamicMeshBatches)
            {
                FVertexFactoryPermutationParameters VertexFactoryParams(Mesh.Element.VertexFactory->GetType(), Mesh.Element.VertexFactory->GetPermutationId());
                FShaderPermutationParameters PermutationParametersVS(&FPreZPassVS::StaticType, 0);
                FShaderPermutationParameters PermutationParametersPS(&FPreZPassPS::StaticType, 0);
                FMeshDrawCommand MeshDrawCommand;
                #ifdef NILOU_DEBUG
                MeshDrawCommand.DebugVertexFactory = Mesh.Element.VertexFactory;
                MeshDrawCommand.DebugMaterial = Mesh.MaterialRenderProxy;
                #endif
                FInputShaderBindings InputBindings = Mesh.Element.Bindings;
                InputBindings.SetElementShaderBinding("FViewShaderParameters", ViewInfo.ViewUniformBuffer->GetHandle());
                BuildMeshDrawCommand(
                    VertexFactoryParams,
                    Mesh.MaterialRenderProxy,
                    PermutationParametersVS,
                    PermutationParametersPS,
                    InputBindings,
                    Mesh.Element.VertexFactory->GetVertexDeclaration(),
                    Mesh.Element,
                    RTLayout,
                    MeshDrawCommand);
                DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
            }

            Views[ViewIndex].MeshDrawCommands = std::move(DrawCommands);
        }
        
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            Graph.AddPass(
                [SceneTextures=ViewSceneTextures[ViewIndex]](RDGPassBuilder& PassBuilder) 
                {
                    PassBuilder.RenderTarget(FA_Depth_Stencil_Attachment, SceneTextures.DepthStencil);
                },
                [=](RHICommandList& RHICmdList) 
                {
                    // FRHIRenderPassInfo PassInfo(SceneTextures->PreZPassFramebuffer.get(), ViewInfo.ScreenResolution, true, true, true);
                    // RHICmdList.RHIBeginRenderPass(PassInfo);
                    FParallelMeshDrawCommands &ViewCommands = Views[ViewIndex].MeshDrawCommands;
                    ViewCommands.DispatchDraw(RHICmdList);
                    // RHICmdList.EndRenderPass();
                }
            );
        }
    }

}