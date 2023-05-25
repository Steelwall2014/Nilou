// #include <glm/gtc/matrix_transform.hpp>
#include "BaseApplication.h"
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

#include "Common/Actor/ReflectionProbe.h"

#ifdef NILOU_DEBUG
#include "CoordinateAxis.h"
#endif


namespace nilou {

    IMPLEMENT_SHADER_TYPE(FScreenQuadVertexShader, "/Shaders/GlobalShaders/ScreenQuadVertexShader.vert", EShaderFrequency::SF_Vertex, Global)
    IMPLEMENT_SHADER_TYPE(FRenderToScreenPixelShader, "/Shaders/GlobalShaders/RenderToScreenPixelShader.frag", EShaderFrequency::SF_Pixel, Global)

    FDefferedShadingSceneRenderer *Renderer = nullptr;

    FSceneRenderer *FSceneRenderer::CreateSceneRenderer(FSceneViewFamily* ViewFamily)
    {
        return new FDefferedShadingSceneRenderer(ViewFamily);
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

    FSceneTextures::FSceneTextures(const SceneTextureCreateInfo &CreateInfo)
    {
        Viewport = CreateInfo.OutputResolution;
        SceneColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "SceneColor", EPixelFormat::PF_R16G16B16A16F, 1, 
            Viewport.x, Viewport.y);

        LightPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();

        DepthStencil = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "DepthStencil", EPixelFormat::PF_D24S8, 1, 
            Viewport.x, Viewport.y);
        
        LightPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, SceneColor);
        LightPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, DepthStencil);
    }

    FSceneTexturesDeffered::FSceneTexturesDeffered(const SceneTextureCreateInfo &CreateInfo)
        : FSceneTextures(CreateInfo)
    {
        GeometryPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();
        PreZPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();

        BaseColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "BaseColor", EPixelFormat::PF_R16G16B16A16F, 1, 
            Viewport.x, Viewport.y);

        RelativeWorldSpacePosition = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "RelativeWorldSpacePosition", EPixelFormat::PF_R16G16B16F, 1, 
            Viewport.x, Viewport.y);

        WorldSpaceNormal = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "WorldSpaceNormal", EPixelFormat::PF_R16G16B16F, 1, 
            Viewport.x, Viewport.y);

        MetallicRoughness = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "MetallicRoughness", EPixelFormat::PF_R16G16F, 1, 
            Viewport.x, Viewport.y);

        Emissive = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "Emissive", EPixelFormat::PF_R16G16B16F, 1, 
            Viewport.x, Viewport.y);

        ShadingModel = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "ShadingModel", EPixelFormat::PF_R8UI, 1, 
            Viewport.x, Viewport.y);

        GeometryPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, BaseColor);
        GeometryPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment1, RelativeWorldSpacePosition);
        GeometryPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment2, WorldSpaceNormal);
        GeometryPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment3, MetallicRoughness);
        GeometryPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment4, Emissive);
        GeometryPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment5, ShadingModel);
        GeometryPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, DepthStencil);
        
        PreZPassFramebuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, DepthStencil);
    }

    FShadowMapTexture::FShadowMapTexture(const ShadowMapResourceCreateInfo& CreateInfo)
    {
        int32 ShadowMapArraySize;
        switch (CreateInfo.LightType) 
        {
        case ELightType::LT_Directional:
            ShadowMapArraySize = CASCADED_SHADOWMAP_SPLIT_COUNT;
            break;
        case ELightType::LT_Point:
            ShadowMapArraySize = 6;
            break;
        case ELightType::LT_Spot:
            ShadowMapArraySize = 1;
            break;
        default:
            return;
        }
        DepthArray = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2DArray(
            "DepthStencil", EPixelFormat::PF_D32FS8, 1, 
            CreateInfo.ShadowMapResolution.x, 
            CreateInfo.ShadowMapResolution.y, 
            ShadowMapArraySize);
        
        for (int i = 0; i < ShadowMapArraySize; i++)
        {
            ShadowMapFramebuffers.push_back(FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer(
                EFramebufferAttachment::FA_Depth_Stencil_Attachment, 
                DepthArray, 
                i));
        }
    }

    FShadowMapUniformBuffer::FShadowMapUniformBuffer(const ShadowMapResourceCreateInfo& CreateInfo)
    {
        switch (CreateInfo.LightType) 
        {
        case ELightType::LT_Directional:
            UniformBuffer = CreateUniformBuffer<FShadowMappingBlock<CASCADED_SHADOWMAP_SPLIT_COUNT>>();
            FrustumCount = CASCADED_SHADOWMAP_SPLIT_COUNT;
            break;
        case ELightType::LT_Point:
            UniformBuffer = CreateUniformBuffer<FShadowMappingBlock<6>>();
            FrustumCount = 6;
            break;
        case ELightType::LT_Spot:
            UniformBuffer = CreateUniformBuffer<FShadowMappingBlock<1>>();
            FrustumCount = 1;
            break;
        }
        UniformBuffer->InitResource();
    }

    FShadowMapResource::FShadowMapResource(const ShadowMapResourceCreateInfo& CreateInfo)
        : ShadowMapTexture(CreateInfo)
        , ShadowMapUniformBuffer(CreateInfo)
    { }

    FSceneRenderer::FScreenQuadPositionVertexBuffer FSceneRenderer::PositionVertexBuffer;
    FSceneRenderer::FScreenQuadUVVertexBuffer FSceneRenderer::UVVertexBuffer;
    FRHIVertexInput FSceneRenderer::PositionVertexInput;
    FRHIVertexInput FSceneRenderer::UVVertexInput;

    FSceneRenderer::TResourcesPool<
        FShadowMapResource, 
        ShadowMapResourceCreateInfo> FSceneRenderer::ShadowMapResourcesPool;

    FSceneRenderer::TResourcesPool<
    FSceneTexturesDeffered, 
    SceneTextureCreateInfo> FSceneRenderer::SceneTexturesPool;

    FSceneRenderer::FSceneRenderer(FSceneViewFamily* InViewFamily)
        : Scene(InViewFamily->Scene)
        , ViewFamily(InViewFamily)
    {
        Views.reserve(InViewFamily->Views.size());
        // MeshCollector.PerViewPDI.resize(InViewFamily->Views.size());
        // MeshCollector.PerViewMeshBatches.resize(InViewFamily->Views.size());
        for(int32 ViewIndex = 0; ViewIndex < InViewFamily->Views.size(); ViewIndex++)
        {
		    FViewInfo& ViewInfo = Views.emplace_back(InViewFamily->Views[ViewIndex]);

            // MeshCollector.PerViewPDI[ViewIndex] = &ViewInfo.PDI;
            // MeshCollector.PerViewMeshBatches[ViewIndex] = &ViewInfo.DynamicMeshBatches;

            ViewFamily.Views[ViewIndex] = &Views[ViewIndex];
        }
    }

    FDefferedShadingSceneRenderer::FDefferedShadingSceneRenderer(FSceneViewFamily* ViewFamily)
        : FSceneRenderer(ViewFamily)
    {

    }


    void FDefferedShadingSceneRenderer::InitViews(FScene *Scene)
    {
        if (!PositionVertexBuffer.IsInitialized())
        {
            PositionVertexBuffer.InitResource();
            UVVertexBuffer.InitResource();
                        
            PositionVertexInput.VertexBuffer = PositionVertexBuffer.VertexBufferRHI.get();
            PositionVertexInput.Location = 0;
            PositionVertexInput.Offset = 0;
            PositionVertexInput.Stride = sizeof(vec4);
            PositionVertexInput.Type = EVertexElementType::VET_Float4;

            UVVertexInput.VertexBuffer = UVVertexBuffer.VertexBufferRHI.get();
            UVVertexInput.Location = 1;
            UVVertexInput.Offset = 0;
            UVVertexInput.Stride = sizeof(vec2);
            UVVertexInput.Type = EVertexElementType::VET_Float2;
        }
        

        Lights.reserve(Scene->AddedLightSceneInfos.size());
        int NumViews = Views.size();
        for (FLightSceneInfo* LightSceneInfo : Scene->AddedLightSceneInfos)
        {
            FLightSceneProxy* Proxy = LightSceneInfo->SceneProxy;
            Lights.emplace_back(Proxy, NumViews, LightSceneInfo->LightUniformBufferRHI);
        }

        
        for(int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
		    FViewInfo& ViewInfo = Views[ViewIndex];
            SceneTextureCreateInfo CreateInfo{ViewInfo.ScreenResolution};
            ViewInfo.SceneTextures = SceneTexturesPool.Alloc(CreateInfo);
        }

        static std::vector<FShadowMapResource*> Resources;
        for (int32 LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
        {
            FLightInfo& Light = Lights[LightIndex];
            ShadowMapResourceCreateInfo CreateInfo{Light.LightSceneProxy->ShadowMapResolution, Light.LightSceneProxy->LightType};

            /** 
             * The number of views being used in shadow mapping.
             * For directional lights, it's the numbder of views.
             * For point/spot lights, it's 1;
             */
            int NumViews = Light.ShadowMapResources.size();
            for (int ViewIndex = 0; ViewIndex < NumViews; ViewIndex++)
            {
                Light.ShadowMapResources[ViewIndex] = ShadowMapResourcesPool.Alloc(CreateInfo);
            }
            LightIndex++;
        }

        // Compute Visibility
        ComputeViewVisibility(Scene, ViewFamily.Views);
    }

    void FDefferedShadingSceneRenderer::ComputeViewVisibility(FScene *Scene, const std::vector<FSceneView*> &SceneViews)
    {
        static UTexture* IBL_BRDF_LUT = GetContentManager()->GetTextureByPath("/Textures/IBL_BRDF_LUT.nasset");
        std::vector<int> Index(SceneViews.size(), 0);
        NILOU_LOG(Info, "Primitive count: {}", Scene->AddedPrimitiveSceneInfos.size())
        for (auto &&PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
        {
            if (!ViewFamily.ShowOnlyComponents.empty() && !ViewFamily.ShowOnlyComponents.contains(PrimitiveInfo->Primitive))
                continue;
            if (ViewFamily.HiddenComponents.contains(PrimitiveInfo->Primitive))
                continue;

            uint32 ViewBits = 0;
            FMeshElementCollector MeshCollector;
            MeshCollector.PerViewPDI.resize(Views.size());
            MeshCollector.PerViewMeshBatches.resize(Views.size());
            for(int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                FViewInfo& ViewInfo = Views[ViewIndex];
                MeshCollector.PerViewPDI[ViewIndex] = &ViewInfo.PDI;
                bool bFrustumCulled = SceneViews[ViewIndex]->ViewFrustum.IsBoxOutSideFrustum(PrimitiveInfo->SceneProxy->GetBounds());
                if (!bFrustumCulled)
                    ViewBits |= (1 << ViewIndex);
            }

            PrimitiveInfo->SceneProxy->GetDynamicMeshElements(SceneViews, ViewBits, MeshCollector);
            
            for (int ViewIndex = 0; ViewIndex < SceneViews.size(); ViewIndex++)
            {
                FViewInfo& ViewInfo = Views[ViewIndex];
                for (FMeshBatch& Mesh : MeshCollector.PerViewMeshBatches[ViewIndex])
                {
                    AReflectionProbe* DefaultProbe = GetAppication()->GetWorld()->SkyboxReflectionProbe;
                    FReflectionProbeSceneProxy* SkyBoxProbeProxy = nullptr;
                    if (DefaultProbe && DefaultProbe->ReflectionProbeComponent)
                        SkyBoxProbeProxy = DefaultProbe->ReflectionProbeComponent->SceneProxy;

                    Mesh.Element.Bindings.SetUniformShaderBinding(
                        "MaterialShadingModel", 
                        (uint32)Mesh.MaterialRenderProxy->ShadingModel);

                    std::vector<std::pair<FReflectionProbeSceneProxy*, float>> ReflectionProbes;
                    switch (PrimitiveInfo->SceneProxy->ReflectionProbeBlendMode)
                    {
                    case RPBM_Off:
                    {
                        if (SkyBoxProbeProxy)
                            ReflectionProbes.push_back({SkyBoxProbeProxy, 1.0});
                        break;
                    }
                    case RPBM_BlendProbes:
                    {   
                        for (auto [ReflectionProbe, factor] : PrimitiveInfo->ReflectionProbeFactors)
                        {
                            ReflectionProbes.push_back({ReflectionProbe->SceneProxy, factor});
                        }
                        break;
                    }
                    case RPBM_BlendProbesAndSkybox:
                    {
                        if (!PrimitiveInfo->ReflectionProbeFactors.empty())
                        {
                            for (auto [ReflectionProbe, factor] : PrimitiveInfo->ReflectionProbeFactors)
                            {
                                ReflectionProbes.push_back({ReflectionProbe->SceneProxy, factor});
                            }
                        }
                        if (SkyBoxProbeProxy)
                        {
                            ReflectionProbes.push_back({SkyBoxProbeProxy, 1.0});
                        }
                        break;
                    }
                    case RPBM_Simple:
                    {
                        // use skybox as fallback
                        FReflectionProbeSceneProxy* MaxFactorProxy = SkyBoxProbeProxy;
                        float MaxFactor = -1;
                        for (auto [ReflectionProbe, factor] : PrimitiveInfo->ReflectionProbeFactors)
                        {
                            if (factor > MaxFactor)
                            {
                                MaxFactor = factor;
                                MaxFactorProxy = ReflectionProbe->SceneProxy;
                            }
                        }
                        ReflectionProbes.push_back({MaxFactorProxy, 1.0});
                        break;
                    }
                    }
                    if (!ReflectionProbes.empty())
                    {
                        for (auto [ReflectionProbe, factor] : ReflectionProbes)
                        {
                            FMeshBatch NewMesh = Mesh;

                            NewMesh.Element.Bindings.SetElementShaderBinding(
                                "IrradianceTexture", 
                                ReflectionProbe->IrradianceTexture);
                            NewMesh.Element.Bindings.SetElementShaderBinding(
                                "PrefilteredTexture", 
                                ReflectionProbe->PrefilteredTexture);
                            NewMesh.Element.Bindings.SetElementShaderBinding(
                                "IBL_BRDF_LUT", 
                                IBL_BRDF_LUT->GetResource()->GetSamplerRHI());
                            NewMesh.Element.Bindings.SetElementShaderBinding(
                                "FViewShaderParameters", 
                                ViewInfo.ViewUniformBuffer->GetRHI());
                            NewMesh.Element.Bindings.SetUniformShaderBinding(
                                "PrefilterEnvTextureNumMips", 
                                static_cast<uint32>(ReflectionProbe->PrefilteredTexture->Texture->GetNumMips()));
                            NewMesh.Element.Bindings.SetUniformShaderBinding(
                                "ReflectionProbeFactor", 
                                factor);
                            
                            Views[ViewIndex].DynamicMeshBatches.push_back(NewMesh);
                        }
                    }
                    else 
                    {
                        Views[ViewIndex].DynamicMeshBatches.push_back(Mesh);
                    }
                }
            }
        }
    }

    void FDefferedShadingSceneRenderer::Render()
    {
        FDynamicRHI *RHICmdList = FDynamicRHI::GetDynamicRHI();

        GetAppication()->GetPreRenderDelegate().Broadcast(RHICmdList, Scene);

        UpdateReflectionProbeFactors();

        InitViews(Scene);

        RenderPreZPass(RHICmdList);

        RenderCSMShadowPass(RHICmdList);

        RenderBasePass(RHICmdList);

        RenderLightingPass(RHICmdList);

        RenderViewElementPass(RHICmdList);

        RenderToScreen(RHICmdList);

        GetAppication()->GetPostRenderDelegate().Broadcast(RHICmdList, Scene);

        ShadowMapResourcesPool.FreeAll();
        SceneTexturesPool.FreeAll();
    }

    float IntersectVolume(const FBoundingBox& box1, const FBoundingBox& box2)
    {
        double xIntersection = std::max(0.0, std::min(box1.Max.x, box2.Max.x) - std::max(box1.Min.x, box2.Min.x));

        if (xIntersection <= 0.0)
            return 0.0;
        

        double yIntersection = std::max(0.0, std::min(box1.Max.y, box2.Max.y) - std::max(box1.Min.y, box2.Min.y));

        if (yIntersection <= 0.0)
            return 0.0;
        

        double zIntersection = std::max(0.0, std::min(box1.Max.z, box2.Max.z) - std::max(box1.Min.z, box2.Min.z));

        if (zIntersection <= 0.0)
            return 0.0;
        

        return xIntersection * yIntersection * zIntersection;
    }

    void FDefferedShadingSceneRenderer::UpdateReflectionProbeFactors()
    {
        for (auto Primitive : Scene->AddedPrimitiveSceneInfos)
        {
            auto& ProbeFactors = Primitive->ReflectionProbeFactors;
            ProbeFactors.clear();
            FBoundingBox PrimitiveExtent = Primitive->SceneProxy->Bounds;
            float total_volume = 0;
            for (auto ReflectionProbe : Scene->ReflectionProbes)
            {
                dvec3 Min = ReflectionProbe->SceneProxy->Location - ReflectionProbe->SceneProxy->Extent/2.0;
                dvec3 Max = ReflectionProbe->SceneProxy->Location + ReflectionProbe->SceneProxy->Extent/2.0;
                FBoundingBox ReflectionProbeExtent(Min, Max);
                float volume = IntersectVolume(PrimitiveExtent, ReflectionProbeExtent);
                if (volume != 0.f)
                {
                    total_volume += volume;
                    ProbeFactors[ReflectionProbe] = volume;
                }
            }
            for (auto& [key, factor] : ProbeFactors)
                factor /= total_volume;
        }
    }
    
    void FDefferedShadingSceneRenderer::RenderToScreen(FDynamicRHI *RHICmdList)
    {
        FTextureRenderTargetResource* RenderTarget = ViewFamily.Viewport.RenderTarget;
        std::vector<RHIFramebuffer*> RenderTargetFramebuffers;
        if (RenderTarget && RenderTarget->TextureType == ETextureType::TT_TextureCube)
        {
            FTextureRenderTargetCubeResource* RenderTargetCube = RenderTarget->GetTextureRenderTargetCubeResource();
            for (int i = 0; i < 6; i++)
            {
                RenderTargetFramebuffers.push_back(RenderTargetCube->RenderTargetFramebuffers[i].get());
            }
        }
        else if (RenderTarget && RenderTarget->TextureType == ETextureType::TT_Texture2D)
        {
            FTextureRenderTarget2DResource* RenderTarget2D = RenderTarget->GetTextureRenderTarget2DResource();
            RenderTargetFramebuffers.push_back(RenderTarget2D->RenderTargetFramebuffer.get());
        }

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            FSceneTextures* SceneTextures = ViewInfo.SceneTextures;
            RHIFramebuffer* OutputRenderTarget = nullptr;
            if (ViewIndex < RenderTargetFramebuffers.size())
                OutputRenderTarget = RenderTargetFramebuffers[ViewIndex];

            FRHIRenderPassInfo PassInfo(OutputRenderTarget, ViewInfo.ScreenResolution, true, true, true);
            RHICmdList->RHIBeginRenderPass(PassInfo);
            {
                
                FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                
                FShaderPermutationParameters PermutationParametersPS(&FRenderToScreenPixelShader::StaticType, 0);

                FShaderInstance *RenderToScreenVS = GetContentManager()->GetGlobalShader(PermutationParametersVS);
                FShaderInstance *RenderToScreenPS = GetContentManager()->GetGlobalShader(PermutationParametersPS);
                
                FGraphicsPipelineStateInitializer PSOInitializer;

                PSOInitializer.VertexShader = RenderToScreenVS->GetVertexShaderRHI();
                PSOInitializer.PixelShader = RenderToScreenPS->GetPixelShaderRHI();

                PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_Triangle_Strip;

                PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::CreateRHI().get();
                PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI().get();
                PSOInitializer.BlendState = TStaticBlendState<>::CreateRHI().get();

                static FRHIVertexInputList VertexInputList = {
                    PositionVertexInput,
                    UVVertexInput
                };
                PSOInitializer.VertexInputList = &VertexInputList;

                FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
                
                RHIGetError();
                RHICmdList->RHISetGraphicsPipelineState(PSO);
                RHIGetError();

                if (ViewFamily.bIsSceneCapture)
                {
                    if (ViewFamily.CaptureSource == SCS_SceneDepth)
                    {
                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "SceneColor", 
                            FRHISampler(SceneTextures->DepthStencil));
                        RHICmdList->RHISetShaderUniformValue(
                            PSO, EPipelineStage::PS_Pixel, 
                            "GammaCorrection", 
                            1.f);
                        RHICmdList->RHISetShaderUniformValue(
                            PSO, EPipelineStage::PS_Pixel, 
                            "bEnableToneMapping", 
                            0);
                    }
                    else if (ViewFamily.CaptureSource == SCS_LinearColor)
                    {
                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "SceneColor", 
                            FRHISampler(SceneTextures->SceneColor));
                        RHICmdList->RHISetShaderUniformValue(
                            PSO, EPipelineStage::PS_Pixel, 
                            "GammaCorrection", 
                            1.f);
                        RHICmdList->RHISetShaderUniformValue(
                            PSO, EPipelineStage::PS_Pixel, 
                            "bEnableToneMapping", 
                            0);
                    }
                    else if (ViewFamily.CaptureSource == SCS_GammaColor)
                    {
                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "SceneColor", 
                            FRHISampler(SceneTextures->SceneColor));
                        RHICmdList->RHISetShaderUniformValue(
                            PSO, EPipelineStage::PS_Pixel, 
                            "GammaCorrection", 
                            ViewFamily.GammaCorrection);
                        RHICmdList->RHISetShaderUniformValue(
                            PSO, EPipelineStage::PS_Pixel, 
                            "bEnableToneMapping", 
                            1);
                    }
                    else 
                    {
                        NILOU_LOG(Error, "Unknown scene capture source");
                    }
                }
                else 
                {
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "SceneColor", 
                        FRHISampler(SceneTextures->SceneColor));
                    RHICmdList->RHISetShaderUniformValue(
                        PSO, EPipelineStage::PS_Pixel, 
                        "GammaCorrection", 
                        ViewFamily.GammaCorrection);
                    RHICmdList->RHISetShaderUniformValue(
                        PSO, EPipelineStage::PS_Pixel, 
                        "bEnableToneMapping", 
                        ViewFamily.bEnableToneMapping);
                }

                RHICmdList->RHIDrawArrays(0, 4);
            }
            RHICmdList->RHIEndRenderPass();
        }


        
    }

    // FSceneTextures* FDefferedShadingSceneRenderer::CreateSceneTextures(const SceneTextureCreateInfo &CreateInfo)
    // {
    //     return new FSceneTexturesDeffered(CreateInfo);
    // }

    // FShadowMapResources* FDefferedShadingSceneRenderer::CreateLightRenderResources(const ShadowMapResourcesCreateInfo &CreateInfo)
    // {
    //     return new FShadowMapResources(CreateInfo);
    // }
}