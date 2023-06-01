#include "PreZPassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DefferedShadingSceneRenderer.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FPreZPassVS, "/Shaders/MaterialShaders/PreZPassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FPreZPassPS, "/Shaders/GlobalShaders/DepthOnlyPixelShader.frag", EShaderFrequency::SF_Pixel, Global);


    static void BuildMeshDrawCommand(
        FDynamicRHI *RHICmdList,
        const FVertexFactoryPermutationParameters &VFPermutationParameters,
        FMaterialRenderProxy *Material,
        const FShaderPermutationParameters &PermutationParametersVS,
        const FShaderPermutationParameters &PermutationParametersPS,
        RHIDepthStencilState* DepthStencilState,
        RHIRasterizerState* RasterizerState,
        RHIBlendState* BlendState,
        FInputShaderBindings &InputBindings,  
        const FRHIVertexInputList* VertexInputs,
        const FMeshBatchElement &Element,
        FMeshDrawCommand &OutMeshDrawCommand
    )
    {
        
        FGraphicsPipelineStateInitializer Initializer;

        FShaderInstance *VertexShader = Material->GetShader(VFPermutationParameters, PermutationParametersVS);
        Initializer.VertexShader = VertexShader->GetVertexShaderRHI();

        FShaderInstance *PixelShader = GetContentManager()->GetGlobalShader(PermutationParametersPS);
        Initializer.PixelShader = PixelShader->GetPixelShaderRHI();

        OutMeshDrawCommand.StencilRef = Material->StencilRefValue;
        Initializer.DepthStencilState = DepthStencilState;
        RHIGetError();

        Initializer.RasterizerState = RasterizerState;
        RHIGetError();

        Initializer.BlendState = BlendState;
        RHIGetError();
        Initializer.VertexInputList = VertexInputs;

        {
            OutMeshDrawCommand.PipelineState = RHICmdList->RHIGetOrCreatePipelineStateObject(Initializer);
            RHIGetError();
            OutMeshDrawCommand.IndexBuffer = Element.IndexBuffer->IndexBufferRHI.get();

            Material->FillShaderBindings(InputBindings);
       
            auto &StageUniformBufferBindings = OutMeshDrawCommand.ShaderBindings.UniformBufferBindings[PS_Vertex]; // alias
            auto &StageSamplerBindings = OutMeshDrawCommand.ShaderBindings.SamplerBindings[PS_Vertex]; // alias
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
                            Material->Name,
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

    void FDefferedShadingSceneRenderer::RenderPreZPass(FDynamicRHI *RHICmdList)
    {

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            FSceneTextures* SceneTextures = ViewInfo.SceneTextures;
            FParallelMeshDrawCommands &DrawCommands = Views[ViewIndex].MeshDrawCommands;
            DrawCommands.Clear();
            // std::vector<FMeshDrawCommand> SkyAtmosphereDrawCommands;
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
                const FRHIVertexInputList* VertexInputs = Mesh.Element.VertexFactory->GetVertexInputList();
                FInputShaderBindings InputBindings = Mesh.Element.Bindings;
                InputBindings.SetElementShaderBinding("FViewShaderParameters", ViewInfo.ViewUniformBuffer->GetRHI());
                BuildMeshDrawCommand(
                    RHICmdList,
                    // *Mesh.MaterialRenderProxy->GetType(),
                    // *Mesh.VertexFactory->GetType(),
                    VertexFactoryParams,
                    Mesh.MaterialRenderProxy,
                    PermutationParametersVS,
                    PermutationParametersPS,
                    Mesh.MaterialRenderProxy->DepthStencilState.get(),
                    Mesh.MaterialRenderProxy->RasterizerState.get(),
                    Mesh.MaterialRenderProxy->BlendState.get(),
                    InputBindings,
                    VertexInputs,
                    Mesh.Element,
                    MeshDrawCommand);
                DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
            }
        }
        
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            FSceneTexturesDeffered* SceneTextures = static_cast<FSceneTexturesDeffered*>(ViewInfo.SceneTextures);
            FRHIRenderPassInfo PassInfo(SceneTextures->PreZPassFramebuffer.get(), ViewInfo.ScreenResolution, true, true, true);
            RHICmdList->RHIBeginRenderPass(PassInfo);

            FParallelMeshDrawCommands &ViewCommands = Views[ViewIndex].MeshDrawCommands;
            
            ViewCommands.DispatchDraw(RHICmdList);
        }
    }

}