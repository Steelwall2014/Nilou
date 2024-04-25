#include "BasePassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DefferedShadingSceneRenderer.h"


namespace nilou {
    IMPLEMENT_SHADER_TYPE(FBasePassVS, "/Shaders/MaterialShaders/BasePassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FBasePassPS, "/Shaders/MaterialShaders/BasePassPixelShader.frag", EShaderFrequency::SF_Pixel, Material);



    static void BuildMeshDrawCommand(
        FDynamicRHI *RHICmdList,
        // const FMaterialType &MaterialType,
        // const FVertexFactoryType &VertexFactoryType,
        const FVertexFactoryPermutationParameters &VFPermutationParameters,
        FMaterialRenderProxy *MaterialProxy,
        const FShaderPermutationParameters &PermutationParametersVS,
        const FShaderPermutationParameters &PermutationParametersPS,
        RHIDepthStencilState* DepthStencilState,
        RHIRasterizerState* RasterizerState,
        RHIBlendState* BlendState,
        FInputShaderBindings &InputBindings, 
        FRHIVertexDeclaration* VertexDeclaration,
        const FMeshBatchElement &Element,
        RHIFramebuffer* Framebuffer,
        FMeshDrawCommand &OutMeshDrawCommand
    )
    {
        FGraphicsPipelineStateInitializer Initializer;

        FShaderInstance *VertexShader = MaterialProxy->GetShader(VFPermutationParameters, PermutationParametersVS);
        Initializer.VertexShader = VertexShader->GetVertexShaderRHI();

        FShaderInstance *PixelShader = MaterialProxy->GetShader(PermutationParametersPS);
        Initializer.PixelShader = PixelShader->GetPixelShaderRHI();

        OutMeshDrawCommand.StencilRef = MaterialProxy->StencilRefValue;
        Initializer.DepthStencilState = DepthStencilState;

        Initializer.RasterizerState = RasterizerState;

        Initializer.BlendState = BlendState;

        Initializer.VertexDeclaration = VertexDeclaration;
        
        Initializer.BuildRenderTargetFormats(Framebuffer);

        {
            OutMeshDrawCommand.VertexStreams = Element.VertexFactory->GetVertexInputStreams();
            OutMeshDrawCommand.PipelineState = RHICmdList->RHIGetOrCreatePipelineStateObject(Initializer);
            RHIGetError();
            OutMeshDrawCommand.IndexBuffer = Element.IndexBuffer->IndexBufferRHI.get();

            MaterialProxy->FillShaderBindings(InputBindings);

            for (int PipelineStage = 0; PipelineStage < EPipelineStage::PipelineStageNum; PipelineStage++)
            {              
                FRHIDescriptorSet &DescriptorSets = OutMeshDrawCommand.PipelineState->PipelineLayout->DescriptorSets[PipelineStage];
                
                for (auto [Name,Binding] : DescriptorSets.Bindings)
                {
                    bool bResourceFound = OutMeshDrawCommand.ShaderBindings.SetShaderBinding(
                        static_cast<EPipelineStage>(PipelineStage), Binding, InputBindings);

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
                            magic_enum::enum_name(static_cast<EPipelineStage>(PipelineStage)),
                            Binding.Name);
                    }

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

    void FDefferedShadingSceneRenderer::RenderBasePass(FDynamicRHI *RHICmdList)
    {    
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            FSceneTextures* SceneTextures = Views[ViewIndex].SceneTextures;
            FParallelMeshDrawCommands &DrawCommands = Views[ViewIndex].MeshDrawCommands;
            DrawCommands.Clear();
            for (FMeshBatch &Mesh : Views[ViewIndex].DynamicMeshBatches)
            {
                FVertexFactoryPermutationParameters VertexFactoryParams(Mesh.Element.VertexFactory->GetType(), Mesh.Element.VertexFactory->GetPermutationId());
                RHIGetError();


                FShaderPermutationParameters PermutationParametersVS(&FBasePassVS::StaticType, 0);
                RHIGetError();
                
                FBasePassPS::FPermutationDomain Domain;
                Domain.Set<FBasePassPS::FDimensionEnableReflectionProbe>((bool)Mesh.bEnableReflectionProbe);
                FShaderPermutationParameters PermutationParametersPS(&FBasePassPS::StaticType, Domain.ToDimensionValueId());
                RHIGetError();


                FMeshDrawCommand MeshDrawCommand;
                #ifdef NILOU_DEBUG
                MeshDrawCommand.DebugVertexFactory = Mesh.Element.VertexFactory;
                MeshDrawCommand.DebugMaterial = Mesh.MaterialRenderProxy;
                #endif

                FSceneTexturesDeffered* SceneTexturesDeffered = static_cast<FSceneTexturesDeffered*>(SceneTextures);

                BuildMeshDrawCommand(
                    RHICmdList,
                    // *Mesh.MaterialRenderProxy->GetType(),
                    // *Mesh.VertexFactory->GetType(),
                    VertexFactoryParams,
                    Mesh.MaterialRenderProxy,
                    PermutationParametersVS,
                    PermutationParametersPS,
                    TStaticDepthStencilState<true, CF_Equal>::CreateRHI().get(),
                    Mesh.MaterialRenderProxy->RasterizerState.get(),
                    Mesh.MaterialRenderProxy->BlendState.get(),
                    Mesh.Element.Bindings,
                    Mesh.Element.VertexFactory->GetVertexDeclaration(),
                    Mesh.Element,
                    SceneTexturesDeffered->GeometryPassFramebuffer.get(),
                    MeshDrawCommand);

                DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                RHIGetError();
                
            }

        }
        

         
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            FSceneTexturesDeffered* SceneTextures = static_cast<FSceneTexturesDeffered*>(ViewInfo.SceneTextures);
            FRHIRenderPassInfo PassInfo(SceneTextures->GeometryPassFramebuffer.get(), ViewInfo.ScreenResolution, true);
            RHICmdList->RHIBeginRenderPass(PassInfo);

            FParallelMeshDrawCommands &ViewCommands = Views[ViewIndex].MeshDrawCommands;
            
            ViewCommands.DispatchDraw(RHICmdList);

            RHICmdList->RHIEndRenderPass();
        }
    


    }
}