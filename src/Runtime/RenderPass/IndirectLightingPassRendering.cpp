#include "IndirectLightingPassRendering.h"
#include "BasePassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DefferedShadingSceneRenderer.h"

namespace nilou {

    IMPLEMENT_SHADER_TYPE(FIndirectLightingPassPS, "/Shaders/MaterialShaders/IndirectLightingPixelShader.frag", EShaderFrequency::SF_Pixel, Material);

    static void BuildMeshDrawCommand(
        FDynamicRHI *RHICmdList,
        // const FMaterialType &MaterialType,
        // const FVertexFactoryType &VertexFactoryType,
        const FVertexFactoryPermutationParameters &VFPermutationParameters,
        FMaterialRenderProxy *Material,
        const FShaderPermutationParameters &PermutationParametersVS,
        const FShaderPermutationParameters &PermutationParametersPS,
        const FDepthStencilStateInitializer &DepthStencilStateInitializer,
        const FRasterizerStateInitializer &RasterizerStateInitializer,
        const FBlendStateInitializer &BlendStateInitializer,
        FInputShaderBindings &InputBindings, 
        std::vector<FRHIVertexInput> &VertexInputs,
        const FMeshBatchElement &Element,
        FMeshDrawCommand &OutMeshDrawCommand
    )
    {
        FRHIGraphicsPipelineInitializer Initializer;

        FShaderInstance *VertexShader = Material->GetShader(VFPermutationParameters, PermutationParametersVS);
        Initializer.VertexShader = VertexShader;

        FShaderInstance *PixelShader = Material->GetShader(PermutationParametersPS);
        Initializer.PixelShader = PixelShader;

        OutMeshDrawCommand.StencilRef = Material->StencilRefValue;
        auto BasePassDepthStencilState = DepthStencilStateInitializer;
        BasePassDepthStencilState.DepthTest = ECompareFunction::CF_Equal;
        OutMeshDrawCommand.DepthStencilState = RHICmdList->RHICreateDepthStencilState(BasePassDepthStencilState);
        RHIGetError();

        OutMeshDrawCommand.RasterizerState = RHICmdList->RHICreateRasterizerState(RasterizerStateInitializer);
        RHIGetError();

        OutMeshDrawCommand.BlendState = RHICmdList->RHICreateBlendState(BlendStateInitializer);
        RHIGetError();

        {
            OutMeshDrawCommand.PipelineState = RHICmdList->RHIGetOrCreatePipelineStateObject(Initializer);
            RHIGetError();
            OutMeshDrawCommand.IndexBuffer = Element.IndexBuffer->IndexBufferRHI.get();

            Material->FillShaderBindings(InputBindings);

            for (int PipelineStage = 0; PipelineStage < EPipelineStage::PipelineStageNum; PipelineStage++)
            {              
                auto &StageUniformBufferBindings = OutMeshDrawCommand.ShaderBindings.UniformBufferBindings[PipelineStage]; // alias
                auto &StageSamplerBindings = OutMeshDrawCommand.ShaderBindings.SamplerBindings[PipelineStage]; // alias
                auto &StageBufferBindings = OutMeshDrawCommand.ShaderBindings.BufferBindings[PipelineStage]; // alias
                FRHIDescriptorSet &DescriptorSets = OutMeshDrawCommand.PipelineState->PipelineLayout.DescriptorSets[PipelineStage];
                
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
                            Material->Name,
                            VFPermutationParameters.Type->Name,
                            PermutationParametersVS.Type->Name,
                            PermutationParametersVS.Type->Name,
                            magic_enum::enum_name(static_cast<EPipelineStage>(PipelineStage)),
                            Binding.Name);
                    }

                }
            }

            OutMeshDrawCommand.ShaderBindings.VertexAttributeBindings = VertexInputs;
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

    void FDefferedShadingSceneRenderer::RenderIndirectLightingPass(FDynamicRHI *RHICmdList)
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

                FShaderPermutationParameters PermutationParametersVS(&FBasePassVS::StaticType, 0);
                
                FShaderPermutationParameters PermutationParametersPS(&FBasePassPS::StaticType, 0);

 
                FMeshDrawCommand MeshDrawCommand;
                #ifdef NILOU_DEBUG
                MeshDrawCommand.DebugVertexFactory = Mesh.Element.VertexFactory;
                MeshDrawCommand.DebugMaterial = Mesh.MaterialRenderProxy;
                #endif
                std::vector<FRHIVertexInput> VertexInputs = Mesh.Element.VertexFactory->GetVertexInputList();
                auto BlendState = Mesh.MaterialRenderProxy->BlendState;

                // Emissive channel is also used as indirect light channel
                // So we need to set its blend state.
                BlendState.bUseIndependentRenderTargetBlendStates = true;
                BlendState.RenderTargets[4].ColorWriteMask = CW_RGB;
                BlendState.RenderTargets[4].ColorBlendOp = EBlendOperation::BO_Add;
                BlendState.RenderTargets[4].ColorDestBlend = EBlendFactor::BF_One;
                BlendState.RenderTargets[4].ColorSrcBlend = EBlendFactor::BF_One;
                
                BuildMeshDrawCommand(
                    RHICmdList,
                    VertexFactoryParams,
                    Mesh.MaterialRenderProxy,
                    PermutationParametersVS,
                    PermutationParametersPS,
                    Mesh.MaterialRenderProxy->DepthStencilState,
                    Mesh.MaterialRenderProxy->RasterizerState,
                    BlendState,
                    Mesh.Element.Bindings,
                    VertexInputs,
                    Mesh.Element,
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
        }
    


    }
}