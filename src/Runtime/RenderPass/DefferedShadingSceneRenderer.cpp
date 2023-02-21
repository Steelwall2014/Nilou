// #include <glm/gtc/matrix_transform.hpp>
#include "Common/BaseApplication.h"
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

#ifdef NILOU_DEBUG
#include "CoordinateAxis.h"
#endif


namespace nilou {

    IMPLEMENT_SHADER_TYPE(FScreenQuadVertexShader, "/Shaders/GlobalShaders/ScreenQuadVertexShader.vert", EShaderFrequency::SF_Vertex, Global)
    IMPLEMENT_SHADER_TYPE(FRenderToScreenPixelShader, "/Shaders/GlobalShaders/RenderToScreenPixelShader.frag", EShaderFrequency::SF_Pixel, Global)

    FDefferedShadingSceneRenderer *Renderer = nullptr;
    
    void CreateSceneTextures(const ivec2 &ScreenResolution, FSceneTextures &OutSceneTextures)
    {
        OutSceneTextures.GeometryPassFrameBuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();
        OutSceneTextures.FrameBuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();

        OutSceneTextures.SceneColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "SceneColor", EPixelFormat::PF_R32G32B32A32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.BaseColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "BaseColor", EPixelFormat::PF_R32G32B32A32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.RelativeWorldSpacePosition = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "RelativeWorldSpacePosition", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.WorldSpaceNormal = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "WorldSpaceNormal", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.MetallicRoughness = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "MetallicRoughness", EPixelFormat::PF_R32G32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.Emissive = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "Emissive", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.DepthStencil = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "DepthStencil", EPixelFormat::PF_D32FS8, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, OutSceneTextures.BaseColor);
        OutSceneTextures.GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment1, OutSceneTextures.RelativeWorldSpacePosition);
        OutSceneTextures.GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment2, OutSceneTextures.WorldSpaceNormal);
        OutSceneTextures.GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment3, OutSceneTextures.MetallicRoughness);
        OutSceneTextures.GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment4, OutSceneTextures.Emissive);
        OutSceneTextures.GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, OutSceneTextures.DepthStencil);
        
        OutSceneTextures.FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, OutSceneTextures.SceneColor);
        OutSceneTextures.FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, OutSceneTextures.DepthStencil);
    }

    FSceneRenderer *FSceneRenderer::CreateSceneRenderer(FScene *Scene)
    {
        return new FDefferedShadingSceneRenderer(Scene);
    }

    void FParallelMeshDrawCommands::AddMeshDrawCommand(const FMeshDrawCommand &MeshDrawCommand)
    {
        MeshCommands.push_back(MeshDrawCommand);
    }
    void FParallelMeshDrawCommands::Clear()
    {
        MeshCommands.clear();
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
        , Collector(&PerViewMeshBatches)
    {
        BeginInitResource(&PositionVertexBuffer);
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

        Scene->GetAddViewDelegate().Add(this, &FDefferedShadingSceneRenderer::OnAddView);
        Scene->GetRemoveViewDelegate().Add(this, &FDefferedShadingSceneRenderer::OnRemoveView);
        Scene->GetResizeViewDelegate().Add(this, &FDefferedShadingSceneRenderer::OnResizeView);
    }

    void FDefferedShadingSceneRenderer::OnAddView(FViewSceneInfo *ViewSceneInfo)
    {
        FSceneTextures SceneTextures;
        CreateSceneTextures(ViewSceneInfo->GetResolution(), SceneTextures);
        Views.emplace_back(ViewSceneInfo, SceneTextures);
        PerViewMeshBatches.push_back(std::vector<FMeshBatch>());
    }

    void FDefferedShadingSceneRenderer::OnRemoveView(FViewSceneInfo *ViewSceneInfo)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            if (Views[ViewIndex].ViewSceneInfo = ViewSceneInfo)
            {
                Views.erase(Views.begin() + ViewIndex);
                PerViewMeshBatches.erase(PerViewMeshBatches.begin() + ViewIndex);
                break;
            }
        }
    }

    void FDefferedShadingSceneRenderer::OnResizeView(FViewSceneInfo *ViewSceneInfo)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            if (Views[ViewIndex].ViewSceneInfo = ViewSceneInfo)
            {
                CreateSceneTextures(ViewSceneInfo->GetResolution(), Views[ViewIndex].SceneTextures);
                break;
            }
        }
    }

    void FDefferedShadingSceneRenderer::InitViews(FScene *Scene)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            PerViewMeshBatches[ViewIndex].clear();
            Views[ViewIndex].MeshDrawCommands.Clear();
        }
        // Compute Visibility
        ComputeViewVisibility(Scene);
    }

    void FDefferedShadingSceneRenderer::ComputeViewVisibility(FScene *Scene)
    {
        std::vector<FViewSceneInfo*> SceneViews;
        for (auto &View : Views)
        {
            SceneViews.push_back(View.ViewSceneInfo);
        }

        for (auto &&PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
        {
            PrimitiveInfo->SceneProxy->GetDynamicMeshElements(SceneViews, 0x7fffffff, Collector);
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
        FDynamicRHI *RHICmdList = FDynamicRHI::GetDynamicRHI();

        InitViews(Scene);

        GetAppication()->GetPreRenderDelegate().Broadcast(RHICmdList);

        // RenderCSMShadowPass(RHICmdList);

        RenderBasePass(RHICmdList);
        // // Dispatch Draw Commands

        RenderLightingPass(RHICmdList);

        RenderAtmospherePass(RHICmdList);

        RenderViewElementPass(RHICmdList);

        RenderToScreen(RHICmdList);

        GetAppication()->GetPostRenderDelegate().Broadcast(RHICmdList);
    }

    // void FDefferedShadingSceneRenderer::RenderCSMShadowPass(FDynamicRHI *RHICmdList)
    // {
    //     for (auto &&LightInfo : Scene->AddedLightSceneInfos)
    //     {
    //         if (LightInfo->SceneProxy->LightType == ELightType::LT_Directional)
    //         {
    //             // glm::vec3 Front = LightInfo->Light->GetForwardVector();
    //             // glm::vec3 Position = LightInfo->Light->GetComponentLocation();
    //             // glm::mat4 ViewMatrix = glm::lookAt(Position, Position + Front, WORLD_UP);
    //             // FSceneLightView LightView;
                
    //         }
    //         else if (LightInfo->SceneProxy->LightType == ELightType::LT_Spot)
    //         {
    //             FSceneLightView LightView;
    //             glm::vec3 Front = LightInfo->SceneProxy->Direction;
    //             glm::vec3 Position = LightInfo->SceneProxy->Position;
    //             float fovy = LightInfo->SceneProxy->ComputedVerticalFieldOfView;
    //             float ScreenAspect = LightInfo->SceneProxy->ScreenAspect;
    //             LightView.ViewMatrix = glm::lookAt(Position, Position + Front, WORLD_UP);
    //             LightView.ProjectionMatrix = glm::perspective(fovy, ScreenAspect, LightInfo->SceneProxy->NearClipDistance, LightInfo->SceneProxy->FarClipDistance);//CalcSpotLightProjectionMatrix(LightInfo->SceneProxy->LightParameters);
    //             LightView.ViewFrustum = FViewFrustum(LightView.ViewMatrix, LightView.ProjectionMatrix);
    //             for (auto &&ViewSceneInfo : Scene->AddedViewSceneInfos)
    //             {
    //                 if (ViewSceneInfo->SceneProxy->GetSceneView().ViewFrustum.Intersects(LightView.ViewFrustum))
    //                     LightInfo->LightViews.push_back(LightView);
    //             }
    //         }
    //     }

    //     std::vector<FParallelMeshDrawCommands> PerLightViewDrawCommands;
    //     for (auto &&LightInfo : Scene->AddedLightSceneInfos)
    //     {
    //         for (FSceneLightView &LightView : LightInfo->LightViews)
    //         {
    //             FParallelMeshDrawCommands ParallelCommands;
    //             for (auto &&PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
    //             {
    //                 // if (LightView.ViewFrustum.IsBoxOutSideFrustum(PrimitiveInfo->SceneProxy->GetBounds()))
    //                 //     continue;
    //                 FMeshBatch Mesh;
    //                 PrimitiveInfo->SceneProxy->GetDynamicMeshElement(Mesh, LightView);
    //                 if (!Mesh.CastShadow)
    //                     continue;
    //                 // FRHIGraphicsPipelineInitializer StateData;

    //                 // FShadowDepthVS::FPermutationDomain PermutationVectorVS;
    //                 // FShaderPermutationParameters PermutationParameters;
    //                 // PermutationParameters.ShaderType = &FShadowDepthVS::StaticType;
    //                 // PermutationParameters.PermutationId = PermutationVectorVS.ToDimensionValueId();
    //                 // FShaderInstance *VertexShader = GetVertexShaderInstance(*FMaterial::GetDefaultMaterial()->GetType(), *Mesh.VertexFactory->GetType(), PermutationParameters);

    //                 // FShadowDepthPS::FPermutationDomain PermutationVectorPS;
    //                 // PermutationParameters.ShaderType = &FShadowDepthPS::StaticType;
    //                 // PermutationParameters.PermutationId = PermutationVectorPS.ToDimensionValueId();
    //                 // FShaderInstance *PixelShader = GetPixelShaderInstance(*FMaterial::GetDefaultMaterial()->GetType(), PermutationParameters);

    //                 // RHIDepthStencilStateRef DepthState = RHICmdList->RHICreateDepthStencilState(Mesh.MaterialRenderProxy->DepthStencilState);
    //                 // StateData.DepthStentilState = DepthState;

    //                 // RHIRasterizerStateRef RasterizerState = RHICmdList->RHICreateRasterizerState(Mesh.MaterialRenderProxy->RasterizerState);
    //                 // StateData.RasterizerState = RasterizerState;

    //                 // std::map<std::string, RHIUniformBuffer *> UniformBufferBindings; 
    //                 // std::map<std::string, FRHISampler *> SamplerBindings; 
    //                 // std::vector<FRHIVertexInput *> VertexInputs; 
    //                 // Mesh.MaterialRenderProxy->FillShaderBindings(UniformBufferBindings, SamplerBindings);
    //                 // Mesh.VertexFactory->FillShaderBindings(UniformBufferBindings, SamplerBindings, VertexInputs);

    //                 // {
    //                 //     FMeshDrawCommand MeshDrawCommand;
    //                 //     MeshDrawCommand.PipelineState = RHICmdList->RHIGetOrCreatePipelineStateObject(StateData);
    //                 //     MeshDrawCommand.IndexBuffer = Mesh.Element.IndexBuffer->IndexBufferRHI.get();
    //                 //     MeshDrawCommand.ShaderBindings.UniformBufferBindings = UniformBufferBindings;
    //                 //     MeshDrawCommand.ShaderBindings.SamplerBindings = SamplerBindings;
    //                 //     MeshDrawCommand.ShaderBindings.VertexAttributeBindings = VertexInputs;
    //                 //     if (Mesh.Element.NumVertices == 0)
    //                 //     {
    //                 //         MeshDrawCommand.IndirectArgs.Buffer = Mesh.Element.IndirectArgsBuffer;
    //                 //         MeshDrawCommand.IndirectArgs.Offset = Mesh.Element.IndirectArgsOffset;
    //                 //         MeshDrawCommand.UseIndirect = true;
    //                 //     }
    //                 //     else
    //                 //     {
    //                 //         MeshDrawCommand.DirectArgs.NumInstances = Mesh.Element.NumInstances;
    //                 //         MeshDrawCommand.DirectArgs.NumVertices = Mesh.Element.NumVertices;
    //                 //         MeshDrawCommand.UseIndirect = false;
    //                 //     }
    //                 //     ParallelCommands.MeshCommands.push_back(MeshDrawCommand);
    //                 // }
    //             }
    //         }
    //     }
    // }



    
    void FDefferedShadingSceneRenderer::RenderToScreen(FDynamicRHI *RHICmdList)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewSceneInfo *CameraInfo = Views[ViewIndex].ViewSceneInfo;
            FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;

            if (CameraInfo->Camera->IsMainCamera())
            {

                FRHIRenderPassInfo PassInfo(nullptr, true, true, true);
                RHICmdList->RHIBeginRenderPass(PassInfo);
                {
                    
                    FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                    
                    FShaderPermutationParameters PermutationParametersPS(&FRenderToScreenPixelShader::StaticType, 0);

                    FShaderInstance *RenderToScreenVS = FContentManager::GetContentManager().GetGlobalShader(PermutationParametersVS);
                    FShaderInstance *RenderToScreenPS = FContentManager::GetContentManager().GetGlobalShader(PermutationParametersPS);
                    
                    FRHIGraphicsPipelineInitializer PSOInitializer;

                    PSOInitializer.VertexShader = RenderToScreenVS;
                    PSOInitializer.PixelShader = RenderToScreenPS;

                    PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_Triangle_Strip;

                    FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
                    
                    RHIDepthStencilStateRef DepthStencilState = TStaticDepthStencilState<false, CF_Always>::CreateRHI();
                    RHIRasterizerStateRef RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI();
                    RHIBlendStateRef BlendState = TStaticBlendState<>::CreateRHI();
                    RHIGetError();
                    RHICmdList->RHISetGraphicsPipelineState(PSO);
                    RHICmdList->RHISetDepthStencilState(DepthStencilState.get());
                    RHICmdList->RHISetRasterizerState(RasterizerState.get());
                    RHICmdList->RHISetBlendState(BlendState.get());
                    RHIGetError();

                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "SceneColor", 
                        FRHISampler(SceneTextures.SceneColor));

                    RHICmdList->RHISetVertexBuffer(PSO, &PositionVertexInput);
                    RHIGetError();
                    RHICmdList->RHISetVertexBuffer(PSO, &UVVertexInput);
                    RHIGetError();
                    RHICmdList->RHIDrawArrays(0, 4);
                }
                RHICmdList->RHIEndRenderPass();

                break;
            }
        }
    }
}