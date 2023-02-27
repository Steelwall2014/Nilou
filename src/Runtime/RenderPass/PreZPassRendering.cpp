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

        FShaderInstance *PixelShader = GetContentManager()->GetGlobalShader(PermutationParametersPS);
        Initializer.PixelShader = PixelShader;

        OutMeshDrawCommand.StencilRef = Material->StencilRefValue;
        OutMeshDrawCommand.DepthStencilState = RHICmdList->RHICreateDepthStencilState(DepthStencilStateInitializer);
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
       
            auto &StageUniformBufferBindings = OutMeshDrawCommand.ShaderBindings.UniformBufferBindings[PS_Vertex]; // alias
            auto &StageSamplerBindings = OutMeshDrawCommand.ShaderBindings.SamplerBindings[PS_Vertex]; // alias
            FRHIDescriptorSet &DescriptorSets = OutMeshDrawCommand.PipelineState->PipelineLayout.DescriptorSets[PS_Vertex];
            
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
                        "Material: " + Material->Name + 
                        " |Vertex Factory: " + VFPermutationParameters.Type->Name + 
                        " |Vertex Shader: " + PermutationParametersVS.Type->Name + 
                        " |Pixel Shader: " + PermutationParametersPS.Type->Name + 
                        " |Pipeline Stage: " + std::to_string(PS_Vertex) + " |\"" + 
                        Binding.Name + "\" Resource not provided");
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

    void FDefferedShadingSceneRenderer::RenderPreZPass(FDynamicRHI *RHICmdList)
    {

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewSceneInfo *ViewInfo = Views[ViewIndex].ViewSceneInfo;
            FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;
            FParallelMeshDrawCommands &DrawCommands = Views[ViewIndex].MeshDrawCommands;
            DrawCommands.Clear();
            std::vector<FMeshDrawCommand> SkyAtmosphereDrawCommands;
            for (FMeshBatch &Mesh : Views[ViewIndex].MeshBatches)
            {
                FVertexFactoryPermutationParameters VertexFactoryParams(Mesh.Element.VertexFactory->GetType(), Mesh.Element.VertexFactory->GetPermutationId());
                FShaderPermutationParameters PermutationParametersVS(&FPreZPassVS::StaticType, 0);
                FShaderPermutationParameters PermutationParametersPS(&FPreZPassPS::StaticType, 0);
                FMeshDrawCommand MeshDrawCommand;
                std::vector<FRHIVertexInput> VertexInputs;
                Mesh.Element.VertexFactory->GetVertexInputList(VertexInputs);
                FInputShaderBindings InputBindings = Mesh.Element.Bindings;
                InputBindings.SetElementShaderBinding("FViewShaderParameters", ViewInfo->SceneProxy->GetViewUniformBuffer());
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
                if (Mesh.MaterialRenderProxy->Name == "SkyAtmosphereMaterial")
                    SkyAtmosphereDrawCommands.push_back(MeshDrawCommand);
                else
                    DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
            }
            for (auto &&DrawCommand : SkyAtmosphereDrawCommands)
                DrawCommands.AddMeshDrawCommand(DrawCommand);
        }
        
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewSceneInfo *CameraInfo = Views[ViewIndex].ViewSceneInfo;
            FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;
            FRHIRenderPassInfo PassInfo(SceneTextures.PreZPassFrameBuffer.get()/*nullptr*/, true, true, true);
            RHICmdList->RHIBeginRenderPass(PassInfo);

            FParallelMeshDrawCommands &ViewCommands = Views[ViewIndex].MeshDrawCommands;
            
            ViewCommands.DispatchDraw(RHICmdList);
        }
    }

}