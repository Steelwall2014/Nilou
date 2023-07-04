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
            Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

        DepthStencil = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "FSceneTextures DepthStencil", EPixelFormat::PF_D24S8, 1, 
            Viewport.x, Viewport.y, TexCreate_DepthStencilTargetable | TexCreate_DepthStencilResolveTarget);

        LightPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer({
            {FA_Color_Attachment0, SceneColor},
            {FA_Depth_Stencil_Attachment, DepthStencil}
        });
    }

    FSceneTexturesDeffered::FSceneTexturesDeffered(const SceneTextureCreateInfo &CreateInfo)
        : FSceneTextures(CreateInfo)
    {

        BaseColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "BaseColor", EPixelFormat::PF_R16G16B16A16F, 1, 
            Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

        RelativeWorldSpacePosition = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "RelativeWorldSpacePosition", EPixelFormat::PF_R16G16B16A16F, 1, 
            Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

        WorldSpaceNormal = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "WorldSpaceNormal", EPixelFormat::PF_R16G16B16A16F, 1, 
            Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

        MetallicRoughness = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "MetallicRoughness", EPixelFormat::PF_R16G16F, 1, 
            Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

        Emissive = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "Emissive", EPixelFormat::PF_R16G16B16A16F, 1, 
            Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

        ShadingModel = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            "ShadingModel", EPixelFormat::PF_R8UI, 1, 
            Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

        GeometryPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer({
            {FA_Color_Attachment0, BaseColor},
            {FA_Color_Attachment1, RelativeWorldSpacePosition},
            {FA_Color_Attachment2, WorldSpaceNormal},
            {FA_Color_Attachment3, MetallicRoughness},
            {FA_Color_Attachment4, Emissive},
            {FA_Color_Attachment5, ShadingModel},
            {FA_Depth_Stencil_Attachment, DepthStencil}
        });
        
        PreZPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer({
            {FA_Depth_Stencil_Attachment, DepthStencil}
        });
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
            "FShadowMapTexture DepthArray", EPixelFormat::PF_D32FS8, 1, 
            CreateInfo.ShadowMapResolution.x, 
            CreateInfo.ShadowMapResolution.y, 
            ShadowMapArraySize, TexCreate_DepthStencilTargetable | TexCreate_DepthStencilResolveTarget);
        
        for (int i = 0; i < ShadowMapArraySize; i++)
        {
            RHITexture2DRef DepthView = FDynamicRHI::GetDynamicRHI()->RHICreateTextureView2D(
                DepthArray.get(), DepthArray->GetFormat(), 0, 1, i);
            DepthViews.push_back(DepthView);
            ShadowMapFramebuffers.push_back(FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer({
                {FA_Depth_Stencil_Attachment, DepthView}
            }));
        }
    }

    FShadowMapUniformBuffer::FShadowMapUniformBuffer(const ShadowMapResourceCreateInfo& CreateInfo)
    {
        switch (CreateInfo.LightType) 
        {
        case ELightType::LT_Directional:
            UniformBuffer = CreateUniformBuffer<FDirectionalShadowMappingBlock>();
            FrustumCount = CASCADED_SHADOWMAP_SPLIT_COUNT;
            break;
        case ELightType::LT_Point:
            UniformBuffer = CreateUniformBuffer<FPointShadowMappingBlock>();
            FrustumCount = 6;
            break;
        case ELightType::LT_Spot:
            UniformBuffer = CreateUniformBuffer<FSpotShadowMappingBlock>();
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
    FRHIVertexDeclaration* FSceneRenderer::ScreenQuadVertexDeclaration;

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
                        
            FVertexInputStream PositionVertexInputStream;
            FVertexElement PositionVertexElement;
            FVertexInputStream UVVertexInputStream;
            FVertexElement UVVertexElement;

            PositionVertexElement.AttributeIndex = 0;
            PositionVertexElement.StreamIndex = 0;
            PositionVertexElement.Offset = 0;
            PositionVertexElement.Stride = sizeof(float) * 4;
            PositionVertexElement.Type = EVertexElementType::VET_Float4;
            PositionVertexInputStream.StreamIndex = 0;
            PositionVertexInputStream.VertexBuffer = PositionVertexBuffer.VertexBufferRHI.get();
            PositionVertexInputStream.Offset = 0;
                        
            UVVertexElement.AttributeIndex = 1;
            UVVertexElement.StreamIndex = 1;
            UVVertexElement.Offset = 0;
            UVVertexElement.Stride = sizeof(float) * 2;
            UVVertexElement.Type = EVertexElementType::VET_Float2;
            UVVertexInputStream.StreamIndex = 1;
            UVVertexInputStream.VertexBuffer = UVVertexBuffer.VertexBufferRHI.get();
            UVVertexInputStream.Offset = 0;

            ScreenQuadVertexDeclaration = FPipelineStateCache::GetOrCreateVertexDeclaration({PositionVertexElement, UVVertexElement});
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

    struct BasePassUniformBlockManager
    {
        static TUniformBufferRef<BasePassPixelShaderUniformBlock> Get()
        {
            if (count == UniformBlocks.size())
            {
                auto ubo = CreateUniformBuffer<BasePassPixelShaderUniformBlock>();
                ubo->InitResource();
                UniformBlocks.push_back(ubo);
            }
            return UniformBlocks[count++];
        }
        static void ReleaseAll()
        {
            count = 0;
        }
        static std::vector<TUniformBufferRef<BasePassPixelShaderUniformBlock>> UniformBlocks;
        static int32 count;
    };
    std::vector<TUniformBufferRef<BasePassPixelShaderUniformBlock>> BasePassUniformBlockManager::UniformBlocks = std::vector<TUniformBufferRef<BasePassPixelShaderUniformBlock>>();
    int32 BasePassUniformBlockManager::count = 0;

    void FDefferedShadingSceneRenderer::ComputeViewVisibility(FScene *Scene, const std::vector<FSceneView*> &SceneViews)
    {
        static UTexture* IBL_BRDF_LUT = GetContentManager()->GetTextureByPath("/Textures/IBL_BRDF_LUT.nasset");
        std::vector<int> Index(SceneViews.size(), 0);
        NILOU_LOG(Info, "Primitive count: {}", Scene->AddedPrimitiveSceneInfos.size())
        BasePassUniformBlockManager::ReleaseAll();

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
                            auto UniformBlock = BasePassUniformBlockManager::Get();
                            UniformBlock->Data.MaterialShadingModel = (uint32)Mesh.MaterialRenderProxy->ShadingModel;
                            UniformBlock->Data.PrefilterEnvTextureNumMips = static_cast<uint32>(ReflectionProbe->PrefilteredTexture->Texture->GetNumMips());
                            UniformBlock->Data.ReflectionProbeFactor = factor;
                            UniformBlock->UpdateUniformBuffer();

                            NewMesh.Element.Bindings.SetElementShaderBinding(
                                "PIXEL_UNIFORM_BLOCK", 
                                UniformBlock->GetRHI());
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
                            // NewMesh.Element.Bindings.SetUniformShaderBinding(
                            //     "PrefilterEnvTextureNumMips", 
                            //     );
                            // NewMesh.Element.Bindings.SetUniformShaderBinding(
                            //     "ReflectionProbeFactor", 
                            //     factor);
                            
                            Views[ViewIndex].DynamicMeshBatches.push_back(NewMesh);
                        }
                    }
                    else 
                    {
                        auto UniformBlock = BasePassUniformBlockManager::Get();
                        UniformBlock->Data.MaterialShadingModel = (uint32)Mesh.MaterialRenderProxy->ShadingModel;
                        UniformBlock->Data.PrefilterEnvTextureNumMips = 1;
                        UniformBlock->Data.ReflectionProbeFactor = 0;
                        UniformBlock->UpdateUniformBuffer();
                        Mesh.Element.Bindings.SetElementShaderBinding(
                            "PIXEL_UNIFORM_BLOCK", 
                            UniformBlock->GetRHI());
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
        static auto UniformBlock = CreateUniformBuffer<FRenderToScreenPixelShader::UniformBlock>();
        UniformBlock->InitResource();
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
            RHIFramebuffer* OutputRenderTarget = RHICmdList->GetRenderToScreenFramebuffer();
            if (ViewIndex < RenderTargetFramebuffers.size())
                OutputRenderTarget = RenderTargetFramebuffers[ViewIndex];

            FRHIRenderPassInfo PassInfo(OutputRenderTarget, ViewInfo.ScreenResolution, true, true, true);
            RHICmdList->RHIBeginRenderPass(PassInfo);
            {
                
                FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                
                FShaderPermutationParameters PermutationParametersPS(&FRenderToScreenPixelShader::StaticType, 0);

                FShaderInstance *RenderToScreenVS = GetGlobalShader(PermutationParametersVS);
                FShaderInstance *RenderToScreenPS = GetGlobalShader(PermutationParametersPS);
                
                FGraphicsPipelineStateInitializer PSOInitializer;

                PSOInitializer.VertexShader = RenderToScreenVS->GetVertexShaderRHI();
                PSOInitializer.PixelShader = RenderToScreenPS->GetPixelShaderRHI();

                PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_TriangleStrip;

                PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::CreateRHI().get();
                PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI().get();
                PSOInitializer.BlendState = TStaticBlendState<>::CreateRHI().get();

                PSOInitializer.VertexDeclaration = ScreenQuadVertexDeclaration;

                PSOInitializer.BuildRenderTargetFormats(OutputRenderTarget);

                FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
                
                RHIGetError();
                RHICmdList->RHISetGraphicsPipelineState(PSO);
                RHIGetError();

                RHICmdList->RHISetStreamSource(0, PositionVertexBuffer.VertexBufferRHI.get(), 0);
                RHICmdList->RHISetStreamSource(1, UVVertexBuffer.VertexBufferRHI.get(), 0);

                if (ViewFamily.bIsSceneCapture)
                {
                    if (ViewFamily.CaptureSource == SCS_SceneDepth)
                    {
                        UniformBlock->Data.GammaCorrection = 1.f;
                        UniformBlock->Data.bEnableToneMapping = 0;
                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "SceneColor", 
                            FRHISampler(SceneTextures->DepthStencil));
                    }
                    else if (ViewFamily.CaptureSource == SCS_LinearColor)
                    {
                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "SceneColor", 
                            FRHISampler(SceneTextures->SceneColor));
                        UniformBlock->Data.GammaCorrection = 1.f;
                        UniformBlock->Data.bEnableToneMapping = 0;
                    }
                    else if (ViewFamily.CaptureSource == SCS_GammaColor)
                    {
                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "SceneColor", 
                            FRHISampler(SceneTextures->SceneColor));
                        UniformBlock->Data.GammaCorrection = ViewFamily.GammaCorrection;
                        UniformBlock->Data.bEnableToneMapping = 1;
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
                    UniformBlock->Data.GammaCorrection = ViewFamily.GammaCorrection;
                    UniformBlock->Data.bEnableToneMapping = ViewFamily.bEnableToneMapping;
                }
                UniformBlock->UpdateUniformBuffer();
                RHICmdList->RHISetShaderUniformBuffer(
                    PSO, EPipelineStage::PS_Pixel, 
                    "PIXEL_UNIFORM_BLOCK", 
                    UniformBlock->GetRHI());

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