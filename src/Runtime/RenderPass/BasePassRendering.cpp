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
                FRHIDescriptorSet &DescriptorSets = OutMeshDrawCommand.PipelineState->PipelineLayout.DescriptorSets[PipelineStage];
                
                for (auto [Name,Binding] : DescriptorSets.Bindings)
                {
                    bool bResourceFound = false;
                    if (Binding.ParameterType == EShaderParameterType::SPT_UniformBuffer)
                    {          
                        if (FUniformBuffer *UniformBuffer = 
                                    InputBindings.GetElementShaderBinding<FUniformBuffer>(Binding.Name))
                        {
                            StageUniformBufferBindings.push_back({Binding.BindingPoint, UniformBuffer->GetRHI()});
                            bResourceFound = true;
                        }
                    }
                    else if (Binding.ParameterType == EShaderParameterType::SPT_Sampler)
                    {  
                        if (FRHISampler *Sampler = 
                                    InputBindings.GetElementShaderBinding<FRHISampler>(Binding.Name))
                        {
                            StageSamplerBindings.push_back({Binding.BindingPoint, Sampler});
                            bResourceFound = true;
                        }
                    }

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

    void FDefferedShadingSceneRenderer::RenderBasePass(FDynamicRHI *RHICmdList)
    {
        // std::map<FViewSceneInfo*, FParallelMeshDrawCommands> PerViewDrawCommands;
        {        
            for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                FViewSceneInfo *CameraInfo = Views[ViewIndex].ViewSceneInfo;
                FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;
                FParallelMeshDrawCommands &DrawCommands = Views[ViewIndex].MeshDrawCommands;
                DrawCommands.Clear();
                // std::vector<FMeshDrawCommand> SkyAtmosphereDrawCommands;
                for (FMeshBatch &Mesh : Views[ViewIndex].MeshBatches)
                {
                    FVertexFactoryPermutationParameters VertexFactoryParams(Mesh.Element.VertexFactory->GetType(), Mesh.Element.VertexFactory->GetPermutationId());
                    RHIGetError();

                    // FMaterialPermutationParameters MaterialParams(Mesh.MaterialRenderProxy->GetType(), Mesh.MaterialRenderProxy->GetPermutationId());

                    FShaderPermutationParameters PermutationParametersVS(&FBasePassVS::StaticType, 0);
                    RHIGetError();
                    
                    FShaderPermutationParameters PermutationParametersPS(&FBasePassPS::StaticType, 0);
                    RHIGetError();

                    FInputShaderBindings InputBindings = Mesh.Element.Bindings;
                    InputBindings.SetElementShaderBinding("FViewShaderParameters", CameraInfo->SceneProxy->GetViewUniformBuffer());
                    RHIGetError();

                    FMeshDrawCommand MeshDrawCommand;
                    std::vector<FRHIVertexInput> VertexInputs;
                    Mesh.Element.VertexFactory->GetVertexInputList(VertexInputs);
                    BuildMeshDrawCommand(
                        RHICmdList,
                        // *Mesh.MaterialRenderProxy->GetType(),
                        // *Mesh.VertexFactory->GetType(),
                        VertexFactoryParams,
                        Mesh.MaterialRenderProxy.get(),
                        PermutationParametersVS,
                        PermutationParametersPS,
                        Mesh.MaterialRenderProxy->DepthStencilState,
                        Mesh.MaterialRenderProxy->RasterizerState,
                        Mesh.MaterialRenderProxy->BlendState,
                        InputBindings,
                        VertexInputs,
                        Mesh.Element,
                        MeshDrawCommand);

                    // SkyAtmosphereMaterial needs to be rendered last
                    // if (Mesh.MaterialRenderProxy->Name == "SkyAtmosphereMaterial")
                    //     SkyAtmosphereDrawCommands.push_back(MeshDrawCommand);
                    // else
                        DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                    RHIGetError();
                    
                }

                // for (auto &&DrawCommand : SkyAtmosphereDrawCommands)
                //     DrawCommands.AddMeshDrawCommand(DrawCommand);
            }
        }

        {        
            for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                FViewSceneInfo *CameraInfo = Views[ViewIndex].ViewSceneInfo;
                FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;
                FRHIRenderPassInfo PassInfo(SceneTextures.GeometryPassFrameBuffer.get(), CameraInfo->GetResolution(), true);
                RHICmdList->RHIBeginRenderPass(PassInfo);

                FParallelMeshDrawCommands &ViewCommands = Views[ViewIndex].MeshDrawCommands;
                
                ViewCommands.DispatchDraw(RHICmdList);
            }
        }


    }
}