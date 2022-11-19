// #include <glm/gtc/matrix_transform.hpp>
#include "Common/Log.h"
#include "UniformBuffer.h"

#include "DefferedShadingSceneRenderer.h"
// #include "Common/Components/LightSceneProxy.h"
// #include "Common/Components/PrimitiveSceneProxy.h"
#include "DynamicRHI.h"
#include "Frustum.h"
#include "Material.h"
#include "ShaderMap.h"

#include "ShadowDepthPassRendering.h"
#include "BasePassRendering.h"
#include "LightingPassRendering.h"

#ifdef _DEBUG
#include "CoordinateAxis.h"
#endif


namespace nilou {

    FSceneRenderer *FSceneRenderer::CreateSceneRenderer(FScene *Scene)
    {
        return new FDefferedShadingSceneRenderer(Scene);
    }

    void FParallelMeshDrawCommands::DispatchDraw(FDynamicRHI *RHICmdList)
    {
        for (auto &&MeshDrawCommand : MeshCommands)
        {
            MeshDrawCommand.SubmitDraw(RHICmdList);
        }
    }

    // FDefferedShadingSceneRenderer::FDefferedShadingSceneRenderer()
    // {
        
    // }

    FDefferedShadingSceneRenderer::FDefferedShadingSceneRenderer(FScene *Scene)
        : Scene(Scene)
    {
        // PositionVertexBuffer.InitRHI();
        BeginInitResource(&PositionVertexBuffer);
        // UVVertexBuffer.InitRHI();
        BeginInitResource(&UVVertexBuffer);
                    
        PositionVertexInput.VertexBuffer = PositionVertexBuffer.VertexBufferRHI.get();
        PositionVertexInput.Location = 0;
        PositionVertexInput.Offset = 0;
        PositionVertexInput.Stride = sizeof(glm::vec4);
        PositionVertexInput.Type = EVertexElementType::VET_Float4;

        UVVertexInput.VertexBuffer = UVVertexBuffer.VertexBufferRHI.get();
        UVVertexInput.Location = 1;
        UVVertexInput.Offset = 0;
        UVVertexInput.Stride = sizeof(glm::vec2);
        UVVertexInput.Type = EVertexElementType::VET_Float2;
    }

    void FDefferedShadingSceneRenderer::ComputeVisibility(FScene *Scene)
    {
        Views.clear();
        for (auto &&View : Scene->AddedCameraSceneInfos)
            Views.push_back(View.get());

        PerViewMeshBatches.resize(Views.size());
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            PerViewMeshBatches[ViewIndex].clear();
            const FSceneView &SceneView = Views[ViewIndex]->SceneProxy->SceneView;
            // FCameraSceneInfo *CameraInfo = Views[ViewIndex];
            // if (CameraInfo->bNeedsUniformBufferUpdate)
            //     CameraInfo->SceneProxy->UpdateUniformBuffer();
            for (auto &&PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
            {
                FMeshBatch Mesh;
                PrimitiveInfo->SceneProxy->GetDynamicMeshElement(Mesh, SceneView);
                Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", PrimitiveInfo->SceneProxy->GetUniformBuffer());
                PerViewMeshBatches[ViewIndex].push_back(Mesh);
            }
        }


        // for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        // {
        //     for (FMeshPassProcessor *Pass : RenderPasses)
        //     {
        //         Pass->BuildMeshDrawCommands(
        //             *Views[ViewIndex], 
        //             PerViewMeshBatches[ViewIndex], 
        //             PerViewDrawCommands[ViewIndex].PerPassMeshCommands[Pass].MeshCommands);
        //     }
        // }
    }

    void FDefferedShadingSceneRenderer::Render()
    {
        FDynamicRHI *RHICmdList = GDynamicRHI;

        // Compute Visibility
        ComputeVisibility(Scene);
        // Now PerViewMeshBatches is filled

        // RenderCSMShadowPass(RHICmdList);

        RenderBasePass(RHICmdList);
        // // Dispatch Draw Commands

        RenderLightingPass(RHICmdList);
    }

    void BuildMeshDrawCommand(
        FDynamicRHI *RHICmdList,
        // const FMaterialType &MaterialType,
        // const FVertexFactoryType &VertexFactoryType,
        const FVertexFactoryPermutationParameters &VFPermutationParameters,
        FMaterial *Material,
        const FShaderPermutationParameters &PermutationParametersVS,
        const FShaderPermutationParameters &PermutationParametersPS,
        const FDepthStencilStateInitializer &DepthStencilStateInitializer,
        const FRasterizerStateInitializer &RasterizerStateInitializer,
        const FBlendStateInitializer &BlendStateInitializer,
        FElementShaderBindings &MeshBindings,    // for vertex factory and material
        FElementShaderBindings &ShaderBindings,  // for shader
        std::vector<FRHIVertexInput> &VertexInputs,
        const FMeshBatchElement &Element,
        FMeshDrawCommand &OutMeshDrawCommand
    )
    {
        FRHIGraphicsPipelineInitializer Initializer;

        // FShaderInstance *VertexShader = GetVertexShaderInstance(MaterialType, VertexFactoryType, PermutationParametersVS);
        FShaderInstance *VertexShader = Material->GetShader(VFPermutationParameters, PermutationParametersVS);//GetVertexShaderInstance2(VFPermutationParameters, MaterialPermutationParameters, PermutationParametersVS);
        Initializer.VertexShader = VertexShader;

        // FShaderInstance *PixelShader = GetPixelShaderInstance(MaterialType, PermutationParametersPS);
        FShaderInstance *PixelShader = Material->GetShader(PermutationParametersPS);//GetPixelShaderInstance2(MaterialPermutationParameters, PermutationParametersPS);
        Initializer.PixelShader = PixelShader;

        OutMeshDrawCommand.DepthStencilState = RHICmdList->RHICreateDepthStencilState(DepthStencilStateInitializer);
            RHICmdList->GLDEBUG();
        // Initializer.DepthStentilState = DepthState;

        OutMeshDrawCommand.RasterizerState = RHICmdList->RHICreateRasterizerState(RasterizerStateInitializer);
            RHICmdList->GLDEBUG();

        OutMeshDrawCommand.BlendState = RHICmdList->RHICreateBlendState(BlendStateInitializer);
            RHICmdList->GLDEBUG();
        // Initializer.RasterizerState = RasterizerState;

        {
            OutMeshDrawCommand.PipelineState = RHICmdList->RHIGetOrCreatePipelineStateObject(Initializer);
            RHICmdList->GLDEBUG();
            OutMeshDrawCommand.IndexBuffer = Element.IndexBuffer->IndexBufferRHI.get();

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
                                    MeshBindings.GetElementShaderBinding<FUniformBuffer>(Binding.Name))
                        {
                            StageUniformBufferBindings.push_back({Binding.BindingPoint, UniformBuffer->GetRHI()});
                            bResourceFound = true;
                        }       
                        else if (FUniformBuffer *UniformBuffer = 
                                    ShaderBindings.GetElementShaderBinding<FUniformBuffer>(Binding.Name))
                        {
                            StageUniformBufferBindings.push_back({Binding.BindingPoint, UniformBuffer->GetRHI()});
                            bResourceFound = true;
                        }
                    }
                    else if (Binding.ParameterType == EShaderParameterType::SPT_Sampler)
                    {  
                        if (FRHISampler *Sampler = 
                                    MeshBindings.GetElementShaderBinding<FRHISampler>(Binding.Name))
                        {
                            StageSamplerBindings.push_back({Binding.BindingPoint, Sampler});
                            bResourceFound = true;
                        }       
                        else if (FRHISampler *Sampler = 
                                    ShaderBindings.GetElementShaderBinding<FRHISampler>(Binding.Name))
                        {
                            StageSamplerBindings.push_back({Binding.BindingPoint, Sampler});
                            bResourceFound = true;
                        }
                    }

                    if (!bResourceFound)
                    {
                        NILOU_LOG(Warning, 
                            "Material: " + Material->GetMaterialName() + 
                            "|Vertex Factory: " + VFPermutationParameters.Type->Name + 
                            "|Vertex Shader: " + PermutationParametersVS.Type->Name + 
                            "|Pixel Shader: " + PermutationParametersPS.Type->Name + 
                            "|Pipeline Stage: " + std::to_string(PipelineStage) + "|\"" + 
                            Binding.Name + "\" Not Found");
                    }

                }
            }

            OutMeshDrawCommand.ShaderBindings.VertexAttributeBindings = &VertexInputs;
            if (Element.NumVertices == 0)
            {
                OutMeshDrawCommand.IndirectArgs.Buffer = Element.IndirectArgsBuffer;
                OutMeshDrawCommand.IndirectArgs.Offset = Element.IndirectArgsOffset;
                OutMeshDrawCommand.UseIndirect = true;
            }
            else
            {
                OutMeshDrawCommand.DirectArgs.NumInstances = Element.NumInstances;
                OutMeshDrawCommand.DirectArgs.NumVertices = Element.NumVertices;
                OutMeshDrawCommand.UseIndirect = false;
            }
        }
    }

    void FDefferedShadingSceneRenderer::RenderBasePass(FDynamicRHI *RHICmdList)
    {
        std::vector<FParallelMeshDrawCommands> PerViewDrawCommands;
        PerViewDrawCommands.resize(Views.size());
        {        
            for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                PerViewDrawCommands[ViewIndex].MeshCommands.clear();
                FCameraSceneInfo *CameraInfo = Views[ViewIndex];
                for (auto &&Mesh : PerViewMeshBatches[ViewIndex])
                {
                    FVertexFactoryPermutationParameters VertexFactoryParams(Mesh.VertexFactory->GetType(), Mesh.VertexFactory->GetPermutationId());
                    GDynamicRHI->GLDEBUG();

                    // FMaterialPermutationParameters MaterialParams(Mesh.MaterialRenderProxy->GetType(), Mesh.MaterialRenderProxy->GetPermutationId());

                    FShaderPermutationParameters PermutationParametersVS(&FBasePassVS::StaticType, 0);
                    GDynamicRHI->GLDEBUG();
                    
                    FShaderPermutationParameters PermutationParametersPS(&FBasePassPS::StaticType, 0);
                    GDynamicRHI->GLDEBUG();

                    FElementShaderBindings Bindings;
                    Bindings.SetElementShaderBinding("FViewShaderParameters", CameraInfo->SceneProxy->GetViewUniformBuffer());
                    GDynamicRHI->GLDEBUG();

                    FMeshDrawCommand MeshDrawCommand;
                    BuildMeshDrawCommand(
                        RHICmdList,
                        // *Mesh.MaterialRenderProxy->GetType(),
                        // *Mesh.VertexFactory->GetType(),
                        VertexFactoryParams,
                        Mesh.MaterialRenderProxy,
                        PermutationParametersVS,
                        PermutationParametersPS,
                        Mesh.MaterialRenderProxy->DepthStencilState,
                        Mesh.MaterialRenderProxy->RasterizerState,
                        Mesh.MaterialRenderProxy->BlendState,
                        Mesh.Element.Bindings,
                        Bindings,
                        Mesh.VertexFactory->GetVertexInputList(),
                        Mesh.Element,
                        MeshDrawCommand);
                    PerViewDrawCommands[ViewIndex].MeshCommands.push_back(MeshDrawCommand);
                    GDynamicRHI->GLDEBUG();
                    
                }
            }
        }

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FRHIRenderPassInfo PassInfo(Views[ViewIndex]->FrameBuffer.get()/*nullptr*/, true, true, true);
            RHICmdList->RHIBeginRenderPass(PassInfo);

            FParallelMeshDrawCommands &ViewCommands = PerViewDrawCommands[ViewIndex];
            
            ViewCommands.DispatchDraw(RHICmdList);
        }

    }

    void FDefferedShadingSceneRenderer::RenderCSMShadowPass(FDynamicRHI *RHICmdList)
    {
        for (auto &&LightInfo : Scene->AddedLightSceneInfos)
        {
            if (LightInfo->SceneProxy->LightType == ELightType::LT_Directional)
            {
                // glm::vec3 Front = LightInfo->Light->GetForwardVector();
                // glm::vec3 Position = LightInfo->Light->GetComponentLocation();
                // glm::mat4 ViewMatrix = glm::lookAt(Position, Position + Front, WORLD_UP);
                // FSceneLightView LightView;
                
            }
            else if (LightInfo->SceneProxy->LightType == ELightType::LT_Spot)
            {
                FSceneLightView LightView;
                glm::vec3 Front = LightInfo->SceneProxy->Direction;
                glm::vec3 Position = LightInfo->SceneProxy->Position;
                float fovy = LightInfo->SceneProxy->ComputedFOVY;
                float ScreenAspect = LightInfo->SceneProxy->ScreenAspect;
                LightView.ViewMatrix = glm::lookAt(Position, Position + Front, WORLD_UP);
                LightView.ProjectionMatrix = glm::perspective(fovy, ScreenAspect, LightInfo->SceneProxy->NearClipDistance, LightInfo->SceneProxy->FarClipDistance);//CalcSpotLightProjectionMatrix(LightInfo->SceneProxy->LightParameters);
                LightView.ViewFrustum = FViewFrustum(LightView.ViewMatrix, LightView.ProjectionMatrix);
                for (auto &&CameraSceneInfo : Scene->AddedCameraSceneInfos)
                {
                    if (CameraSceneInfo->SceneProxy->SceneView.ViewFrustum.Intersects(LightView.ViewFrustum))
                        LightInfo->LightViews.push_back(LightView);
                }
            }
        }

        std::vector<FParallelMeshDrawCommands> PerLightViewDrawCommands;
        for (auto &&LightInfo : Scene->AddedLightSceneInfos)
        {
            for (FSceneLightView &LightView : LightInfo->LightViews)
            {
                FParallelMeshDrawCommands ParallelCommands;
                for (auto &&PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
                {
                    // if (LightView.ViewFrustum.IsBoxOutSideFrustum(PrimitiveInfo->SceneProxy->GetBounds()))
                    //     continue;
                    FMeshBatch Mesh;
                    PrimitiveInfo->SceneProxy->GetDynamicMeshElement(Mesh, LightView);
                    if (!Mesh.CastShadow)
                        continue;
                    // FRHIGraphicsPipelineInitializer StateData;

                    // FShadowDepthVS::FPermutationDomain PermutationVectorVS;
                    // FShaderPermutationParameters PermutationParameters;
                    // PermutationParameters.ShaderType = &FShadowDepthVS::StaticType;
                    // PermutationParameters.PermutationId = PermutationVectorVS.ToDimensionValueId();
                    // FShaderInstance *VertexShader = GetVertexShaderInstance(*FMaterial::GetDefaultMaterial()->GetType(), *Mesh.VertexFactory->GetType(), PermutationParameters);

                    // FShadowDepthPS::FPermutationDomain PermutationVectorPS;
                    // PermutationParameters.ShaderType = &FShadowDepthPS::StaticType;
                    // PermutationParameters.PermutationId = PermutationVectorPS.ToDimensionValueId();
                    // FShaderInstance *PixelShader = GetPixelShaderInstance(*FMaterial::GetDefaultMaterial()->GetType(), PermutationParameters);

                    // RHIDepthStencilStateRef DepthState = RHICmdList->RHICreateDepthStencilState(Mesh.MaterialRenderProxy->DepthStencilState);
                    // StateData.DepthStentilState = DepthState;

                    // RHIRasterizerStateRef RasterizerState = RHICmdList->RHICreateRasterizerState(Mesh.MaterialRenderProxy->RasterizerState);
                    // StateData.RasterizerState = RasterizerState;

                    // std::map<std::string, RHIUniformBuffer *> UniformBufferBindings; 
                    // std::map<std::string, FRHISampler *> SamplerBindings; 
                    // std::vector<FRHIVertexInput *> VertexInputs; 
                    // Mesh.MaterialRenderProxy->CollectShaderBindings(UniformBufferBindings, SamplerBindings);
                    // Mesh.VertexFactory->CollectShaderBindings(UniformBufferBindings, SamplerBindings, VertexInputs);

                    // {
                    //     FMeshDrawCommand MeshDrawCommand;
                    //     MeshDrawCommand.PipelineState = RHICmdList->RHIGetOrCreatePipelineStateObject(StateData);
                    //     MeshDrawCommand.IndexBuffer = Mesh.Element.IndexBuffer->IndexBufferRHI.get();
                    //     MeshDrawCommand.ShaderBindings.UniformBufferBindings = UniformBufferBindings;
                    //     MeshDrawCommand.ShaderBindings.SamplerBindings = SamplerBindings;
                    //     MeshDrawCommand.ShaderBindings.VertexAttributeBindings = VertexInputs;
                    //     if (Mesh.Element.NumVertices == 0)
                    //     {
                    //         MeshDrawCommand.IndirectArgs.Buffer = Mesh.Element.IndirectArgsBuffer;
                    //         MeshDrawCommand.IndirectArgs.Offset = Mesh.Element.IndirectArgsOffset;
                    //         MeshDrawCommand.UseIndirect = true;
                    //     }
                    //     else
                    //     {
                    //         MeshDrawCommand.DirectArgs.NumInstances = Mesh.Element.NumInstances;
                    //         MeshDrawCommand.DirectArgs.NumVertices = Mesh.Element.NumVertices;
                    //         MeshDrawCommand.UseIndirect = false;
                    //     }
                    //     ParallelCommands.MeshCommands.push_back(MeshDrawCommand);
                    // }
                }
            }
        }
    }

    void FDefferedShadingSceneRenderer::RenderLightingPass(FDynamicRHI *RHICmdList)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FCameraSceneInfo *CameraInfo = Views[ViewIndex];
            if (CameraInfo->Camera->IsMainCamera())
            {

                FRHIRenderPassInfo PassInfo(nullptr, true, true, true);
                RHICmdList->RHIBeginRenderPass(PassInfo);
                {
                    
                    FShaderPermutationParameters PermutationParametersVS(&FLightingPassVS::StaticType, 0);
                    
                    FLightingPassPS::FPermutationDomain PermutationVectorPS;
                    // PermutationVectorPS.Set<FLightingPassPS::FDimentionLightNum>(Scene->AddedLightSceneInfos.size());
                    FShaderPermutationParameters PermutationParametersPS(&FLightingPassPS::StaticType, PermutationVectorPS.ToDimensionValueId());

                    FShaderInstance *LightPassVS = GetGlobalShaderInstance2(PermutationParametersVS);
                    FShaderInstance *LightPassPS = GetGlobalShaderInstance2(PermutationParametersPS);
                    
                    FRHIGraphicsPipelineInitializer PSOInitializer;

                    PSOInitializer.VertexShader = LightPassVS;
                    PSOInitializer.PixelShader = LightPassPS;

                    PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_Triangle_Strip;

                    FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
                    
                    RHIDepthStencilStateRef DepthStencilState = TStaticDepthStencilState<true, CF_Always>::CreateRHI();
                    RHIRasterizerStateRef RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI();
                    RHIBlendStateRef BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::CreateRHI();
                    RHICmdList->GLDEBUG();
                    RHICmdList->RHISetGraphicsPipelineState(PSO);
                    RHICmdList->RHISetDepthStencilState(DepthStencilState.get());
                    RHICmdList->RHISetRasterizerState(RasterizerState.get());
                    RHICmdList->RHISetBlendState(BlendState.get());
                    RHICmdList->GLDEBUG();

                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "BaseColor", 
                        FRHISampler(CameraInfo->SceneTextures.BaseColor));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "WorldSpacePosition", 
                        FRHISampler(CameraInfo->SceneTextures.WorldSpacePosition));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "WorldSpaceNormal", 
                        FRHISampler(CameraInfo->SceneTextures.WorldSpaceNormal));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "MetallicRoughness", 
                        FRHISampler(CameraInfo->SceneTextures.MetallicRoughness));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "Emissive", 
                        FRHISampler(CameraInfo->SceneTextures.Emissive));
                    RHICmdList->GLDEBUG();
                    RHICmdList->RHISetShaderUniformBuffer(
                        PSO, EPipelineStage::PS_Pixel, 
                        "FViewShaderParameters", 
                        CameraInfo->SceneProxy->GetViewUniformBuffer()->GetRHI());
                    RHICmdList->GLDEBUG();

                    RHICmdList->RHISetVertexBuffer(PSO, &PositionVertexInput);
                    RHICmdList->GLDEBUG();
                    RHICmdList->RHISetVertexBuffer(PSO, &UVVertexInput);
                    RHICmdList->GLDEBUG();
                    for (auto &&LightInfo : Scene->AddedLightSceneInfos)
                    {
                        RHICmdList->RHISetShaderUniformBuffer(
                            PSO, EPipelineStage::PS_Pixel, 
                            "FLightUniformBlock", 
                            LightInfo->SceneProxy->LightUniformBufferRHI->GetRHI());

                        RHICmdList->RHIDrawArrays(4);
                    }
                }
                RHICmdList->RHIEndRenderPass();

                break;
            }
        }
    }
}