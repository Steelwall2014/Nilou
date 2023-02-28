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
    
    FSceneTextures::FSceneTextures(const ivec2 &ScreenResolution)
    {
        Viewport = ScreenResolution;
        GeometryPassFrameBuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();
        FrameBuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();
        PreZPassFrameBuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();

        SceneColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "SceneColor", EPixelFormat::PF_R32G32B32A32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        BaseColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "BaseColor", EPixelFormat::PF_R32G32B32A32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        RelativeWorldSpacePosition = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "RelativeWorldSpacePosition", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        WorldSpaceNormal = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "WorldSpaceNormal", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        MetallicRoughness = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "MetallicRoughness", EPixelFormat::PF_R32G32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        Emissive = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "Emissive", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        DepthStencil = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "DepthStencil", EPixelFormat::PF_D32FS8, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, BaseColor);
        GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment1, RelativeWorldSpacePosition);
        GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment2, WorldSpaceNormal);
        GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment3, MetallicRoughness);
        GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment4, Emissive);
        GeometryPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, DepthStencil);
        
        FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, SceneColor);
        FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, DepthStencil);
        
        PreZPassFrameBuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, DepthStencil);
    }
    
    FShadowMapTextures::FShadowMapTextures(const ivec2 &ShadowMapResolution, int ShadowMapArraySize)
    {
        DepthArray = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2DArray(
            "DepthStencil", EPixelFormat::PF_D32FS8, 1, 
            ShadowMapResolution.x, ShadowMapResolution.y, ShadowMapArraySize, nullptr);
        
        for (int i = 0; i < ShadowMapArraySize; i++)
        {
            FrameBuffers.push_back(FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer(
                EFramebufferAttachment::FA_Depth_Stencil_Attachment, 
                DepthArray, 
                i));
        }
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
        for (int i = 0; i < MeshCommands.size(); i++)
        {
            auto &&MeshDrawCommand = MeshCommands[i];
            MeshDrawCommand.SubmitDraw(RHICmdList);
        }
    }

    // FDefferedShadingSceneRenderer::FDefferedShadingSceneRenderer()
    // {
        
    // }

    FDefferedShadingSceneRenderer::FDefferedShadingSceneRenderer(FScene *Scene)
        : Scene(Scene)
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

        Scene->GetAddLightDelegate().Add(this, &FDefferedShadingSceneRenderer::OnAddLight);
        Scene->GetRemoveLightDelegate().Add(this, &FDefferedShadingSceneRenderer::OnRemoveLight);
    }

    void FDefferedShadingSceneRenderer::OnAddView(FViewSceneInfo *ViewSceneInfo)
    {
        Views.emplace_back(ViewSceneInfo, FSceneTextures(ViewSceneInfo->GetResolution()));
        for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
        {
            FLightSceneInfo *LightSceneInfo = Lights[LightIndex].LightSceneInfo;
            if (LightSceneInfo->SceneProxy->LightType == ELightType::LT_Directional)
            {
                Lights[LightIndex].ShadowMapTextures.push_back(FShadowMapTextures(LightSceneInfo->SceneProxy->ShadowMapResolution, 8));
                Lights[LightIndex].ShadowMapUniformBuffers.push_back(FShadowMapUniformBuffers::Create<CASCADED_SHADOWMAP_SPLIT_COUNT>());
                Lights[LightIndex].ShadowMapMeshBatches.push_back(FShadowMapMeshBatches(8));
                Lights[LightIndex].ShadowMapMeshDrawCommands.push_back(FShadowMapMeshDrawCommands(8));
            }
            else if (LightSceneInfo->SceneProxy->LightType == ELightType::LT_Point) 
            {
                Lights[LightIndex].ShadowMapTextures.push_back(FShadowMapTextures(LightSceneInfo->SceneProxy->ShadowMapResolution, 6));
                Lights[LightIndex].ShadowMapUniformBuffers.push_back(FShadowMapUniformBuffers::Create<6>());
                Lights[LightIndex].ShadowMapMeshBatches.push_back(FShadowMapMeshBatches(6));
                Lights[LightIndex].ShadowMapMeshDrawCommands.push_back(FShadowMapMeshDrawCommands(6));
            }
            else if (LightSceneInfo->SceneProxy->LightType == ELightType::LT_Spot) 
            {
                Lights[LightIndex].ShadowMapTextures.push_back(FShadowMapTextures(LightSceneInfo->SceneProxy->ShadowMapResolution, 1));
                Lights[LightIndex].ShadowMapUniformBuffers.push_back(FShadowMapUniformBuffers::Create<1>());
                Lights[LightIndex].ShadowMapMeshBatches.push_back(FShadowMapMeshBatches(1));
                Lights[LightIndex].ShadowMapMeshDrawCommands.push_back(FShadowMapMeshDrawCommands(1));
            }
        }
    }

    void FDefferedShadingSceneRenderer::OnRemoveView(FViewSceneInfo *ViewSceneInfo)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            if (Views[ViewIndex].ViewSceneInfo = ViewSceneInfo)
            {
                Views.erase(Views.begin() + ViewIndex);

                for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
                {
                    Lights[LightIndex].ShadowMapTextures.erase(Lights[LightIndex].ShadowMapTextures.begin() + ViewIndex);
                    Lights[LightIndex].ShadowMapUniformBuffers.erase(Lights[LightIndex].ShadowMapUniformBuffers.begin() + ViewIndex);
                    Lights[LightIndex].ShadowMapMeshBatches.erase(Lights[LightIndex].ShadowMapMeshBatches.begin() + ViewIndex);
                    Lights[LightIndex].ShadowMapMeshDrawCommands.erase(Lights[LightIndex].ShadowMapMeshDrawCommands.begin() + ViewIndex);
                }
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
                Views[ViewIndex].SceneTextures = FSceneTextures(ViewSceneInfo->GetResolution());
                break;
            }
        }
    }

    void FDefferedShadingSceneRenderer::OnAddLight(FLightSceneInfo *LightInfo)
    {
        auto &Light = Lights.emplace_back(LightInfo);
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            if (LightInfo->SceneProxy->LightType == ELightType::LT_Directional)
            {
                Light.ShadowMapTextures.push_back(FShadowMapTextures(LightInfo->SceneProxy->ShadowMapResolution, 8));
                Light.ShadowMapUniformBuffers.push_back(FShadowMapUniformBuffers::Create<CASCADED_SHADOWMAP_SPLIT_COUNT>());
                Light.ShadowMapMeshBatches.push_back(FShadowMapMeshBatches(CASCADED_SHADOWMAP_SPLIT_COUNT));
                Light.ShadowMapMeshDrawCommands.push_back(FShadowMapMeshDrawCommands(CASCADED_SHADOWMAP_SPLIT_COUNT));
            }
            else if (LightInfo->SceneProxy->LightType == ELightType::LT_Point) 
            {
                Light.ShadowMapTextures.push_back(FShadowMapTextures(LightInfo->SceneProxy->ShadowMapResolution, 6));
                Light.ShadowMapUniformBuffers.push_back(FShadowMapUniformBuffers::Create<6>());
                Light.ShadowMapMeshBatches.push_back(FShadowMapMeshBatches(6));
                Light.ShadowMapMeshDrawCommands.push_back(FShadowMapMeshDrawCommands(6));
            }
            else if (LightInfo->SceneProxy->LightType == ELightType::LT_Spot) 
            {
                Light.ShadowMapTextures.push_back(FShadowMapTextures(LightInfo->SceneProxy->ShadowMapResolution, 1));
                Light.ShadowMapUniformBuffers.push_back(FShadowMapUniformBuffers::Create<1>());
                Light.ShadowMapMeshBatches.push_back(FShadowMapMeshBatches(1));
                Light.ShadowMapMeshDrawCommands.push_back(FShadowMapMeshDrawCommands(1));
            }
        }
    }

    void FDefferedShadingSceneRenderer::OnRemoveLight(FLightSceneInfo *LightInfo)
    {
        for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
        {
            if (Lights[LightIndex].LightSceneInfo = LightInfo)
            {
                Lights.erase(Lights.begin() + LightIndex);
                break;
            }
        }
    }


    void FDefferedShadingSceneRenderer::InitViews(FScene *Scene)
    {
        Collector.PerViewMeshBatches.clear();
        Collector.PerViewPDI.clear();
        std::vector<FSceneView> SceneViews;
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            Views[ViewIndex].MeshBatches.clear();
            Views[ViewIndex].MeshDrawCommands.Clear();
            SceneViews.push_back(Views[ViewIndex].ViewSceneInfo->SceneProxy->GetSceneView());
            Collector.PerViewMeshBatches.push_back(&Views[ViewIndex].MeshBatches);
            Collector.PerViewPDI.push_back(Views[ViewIndex].PDI);
        }

        // Compute Visibility
        ComputeViewVisibility(Scene, SceneViews);
    }

    void FDefferedShadingSceneRenderer::ComputeViewVisibility(FScene *Scene, const std::vector<FSceneView> &SceneViews)
    {
        for (auto &&PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
        {
            uint32 ViewBits = 0;
            for (int ViewIndex = 0; ViewIndex < SceneViews.size(); ViewIndex++)
            {
                bool bFrustumCulled = SceneViews[ViewIndex].ViewFrustum.IsBoxOutSideFrustum(PrimitiveInfo->SceneProxy->GetBounds());
                if (!bFrustumCulled)
                    ViewBits |= (1 << ViewIndex);
            }
            PrimitiveInfo->SceneProxy->GetDynamicMeshElements(SceneViews, ViewBits, Collector);
        }
        // std::vector<FSceneView> LightSceneViews;
        // for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++) 
        // {
        //     Lights[LightIndex].MeshBatches.clear();
        //     Lights[LightIndex].MeshDrawCommands.Clear();
        //     LightSceneViews.push_back(Lights[LightIndex].LightSceneInfo->SceneProxy);
        //     LightCollector.PerViewMeshBatches.push_back(&Lights[LightIndex].MeshBatches);
        // }
    }

    void FDefferedShadingSceneRenderer::Render()
    {
        FDynamicRHI *RHICmdList = FDynamicRHI::GetDynamicRHI();

        InitViews(Scene);

        GetAppication()->GetPreRenderDelegate().Broadcast(RHICmdList);

        RenderPreZPass(RHICmdList);

        RenderCSMShadowPass(RHICmdList);

        RenderBasePass(RHICmdList);
        // // Dispatch Draw Commands

        RenderLightingPass(RHICmdList);

        RenderAtmospherePass(RHICmdList);

        RenderViewElementPass(RHICmdList);

        RenderToScreen(RHICmdList);

        GetAppication()->GetPostRenderDelegate().Broadcast(RHICmdList);
    }


    
    void FDefferedShadingSceneRenderer::RenderToScreen(FDynamicRHI *RHICmdList)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewSceneInfo *CameraInfo = Views[ViewIndex].ViewSceneInfo;
            FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;

            if (CameraInfo->Camera->IsMainCamera())
            {

                FRHIRenderPassInfo PassInfo(nullptr, CameraInfo->GetResolution(), true, true, true);
                RHICmdList->RHIBeginRenderPass(PassInfo);
                {
                    
                    FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                    
                    FShaderPermutationParameters PermutationParametersPS(&FRenderToScreenPixelShader::StaticType, 0);

                    FShaderInstance *RenderToScreenVS = GetContentManager()->GetGlobalShader(PermutationParametersVS);
                    FShaderInstance *RenderToScreenPS = GetContentManager()->GetGlobalShader(PermutationParametersPS);
                    
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