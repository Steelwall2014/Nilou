// #include <glm/gtc/matrix_transform.hpp>
#include "BaseApplication.h"
#include "Common/Log.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

#include "DeferredShadingSceneRenderer.h"
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

    FDeferredShadingSceneRenderer *Renderer = nullptr;

    FSceneRenderer *FSceneRenderer::CreateSceneRenderer(FSceneViewFamily& ViewFamily)
    {
        return new FDeferredShadingSceneRenderer(ViewFamily);
    }

    void FParallelMeshDrawCommands::AddMeshDrawCommand(const FMeshDrawCommand &MeshDrawCommand)
    {
        MeshCommands.push_back(MeshDrawCommand);
    }
    void FParallelMeshDrawCommands::Clear()
    {
        MeshCommands.clear();
    }
    void FParallelMeshDrawCommands::DispatchDraw(RHICommandList& RHICmdList) const
    {
        for (int i = 0; i < MeshCommands.size(); i++)
        {
            const FMeshDrawCommand& MeshDrawCommand = MeshCommands[i];
            MeshDrawCommand.SubmitDraw(RHICmdList);
        }
    }

    // FSceneTextures::FSceneTextures(const SceneTextureCreateInfo &CreateInfo)
    // {
    //     Viewport = CreateInfo.OutputResolution;
    //     SceneColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
    //         "SceneColor", EPixelFormat::PF_R16G16B16A16F, 1, 
    //         Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

    //     DepthStencil = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
    //         "FSceneTextures DepthStencil", EPixelFormat::PF_D24S8, 1, 
    //         Viewport.x, Viewport.y, TexCreate_DepthStencilTargetable | TexCreate_DepthStencilResolveTarget);

    //     LightPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer({
    //         {FA_Color_Attachment0, SceneColor},
    //         {FA_Depth_Stencil_Attachment, DepthStencil}
    //     });

    //     BaseColor = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
    //         "BaseColor", EPixelFormat::PF_R16G16B16A16F, 1, 
    //         Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

    //     RelativeWorldSpacePosition = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
    //         "RelativeWorldSpacePosition", EPixelFormat::PF_R16G16B16A16F, 1, 
    //         Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

    //     WorldSpaceNormal = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
    //         "WorldSpaceNormal", EPixelFormat::PF_R16G16B16A16F, 1, 
    //         Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

    //     MetallicRoughness = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
    //         "MetallicRoughness", EPixelFormat::PF_R16G16F, 1, 
    //         Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

    //     Emissive = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
    //         "Emissive", EPixelFormat::PF_R16G16B16A16F, 1, 
    //         Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

    //     ShadingModel = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
    //         "ShadingModel", EPixelFormat::PF_R8UI, 1, 
    //         Viewport.x, Viewport.y, TexCreate_RenderTargetable | TexCreate_ResolveTargetable);

    //     GeometryPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer({
    //         {FA_Color_Attachment0, BaseColor},
    //         {FA_Color_Attachment1, RelativeWorldSpacePosition},
    //         {FA_Color_Attachment2, WorldSpaceNormal},
    //         {FA_Color_Attachment3, MetallicRoughness},
    //         {FA_Color_Attachment4, Emissive},
    //         {FA_Color_Attachment5, ShadingModel},
    //         {FA_Depth_Stencil_Attachment, DepthStencil}
    //     });
        
    //     PreZPassFramebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer({
    //         {FA_Depth_Stencil_Attachment, DepthStencil}
    //     });
    // }

    FSceneRenderer::FScreenQuadPositionVertexBuffer FSceneRenderer::PositionVertexBuffer;
    FSceneRenderer::FScreenQuadUVVertexBuffer FSceneRenderer::UVVertexBuffer;
    FRHIVertexDeclaration* FSceneRenderer::ScreenQuadVertexDeclaration;

    // FSceneRenderer::TResourcesPool<
    //     FShadowMapResource, 
    //     ShadowMapResourceCreateInfo> FSceneRenderer::ShadowMapResourcesPool;

    // FSceneRenderer::TResourcesPool<
    // FSceneTexturesDeferred, 
    // SceneTextureCreateInfo> FSceneRenderer::SceneTexturesPool;

    FSceneRenderer::FSceneRenderer(FSceneViewFamily& InViewFamily)
        : Scene(InViewFamily.Scene)
        , ViewFamily(InViewFamily)
        , Views(InViewFamily.Views)
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
            PositionVertexInputStream.VertexBuffer = PositionVertexBuffer.VertexBufferRDG.get();
            PositionVertexInputStream.Offset = 0;
                        
            UVVertexElement.AttributeIndex = 1;
            UVVertexElement.StreamIndex = 1;
            UVVertexElement.Offset = 0;
            UVVertexElement.Stride = sizeof(float) * 2;
            UVVertexElement.Type = EVertexElementType::VET_Float2;
            UVVertexInputStream.StreamIndex = 1;
            UVVertexInputStream.VertexBuffer = UVVertexBuffer.VertexBufferRDG.get();
            UVVertexInputStream.Offset = 0;

            ScreenQuadVertexDeclaration = RHICreateVertexDeclaration({PositionVertexElement, UVVertexElement});
        }
    }

    FDeferredShadingSceneRenderer::FDeferredShadingSceneRenderer(FSceneViewFamily& ViewFamily)
        : FSceneRenderer(ViewFamily)
    {

    }


    void FDeferredShadingSceneRenderer::InitViews(RenderGraph& Graph)
    {
        // Initialize lights
        // Lights are relavant to views, for example directional lights.
        Lights.reserve(Scene->AddedLightSceneInfos.size());
        for (FLightSceneInfo* LightSceneInfo : Scene->AddedLightSceneInfos)
        {
            FLightSceneProxy* Proxy = LightSceneInfo->SceneProxy;
            FLightInfo LightInfo;
            LightInfo.LightType = Proxy->LightType;
            LightInfo.LightUniformBuffer = LightSceneInfo->LightUniformBuffer.get();
            int NumRelevantViews = 1;
            if (Proxy->LightType == ELightType::LT_Directional)
                NumRelevantViews = Views.size();
            for (int ViewIndex = 0; ViewIndex < NumRelevantViews; ViewIndex++)
            {
                FShadowMapResource Resource;
                RDGBufferDesc BufferDesc;
                RDGTextureDesc TextureDesc;
                TextureDesc.SizeX = 1024;
                TextureDesc.SizeY = 1024;
                TextureDesc.NumMips = 1;
                TextureDesc.Format = EPixelFormat::PF_D24S8;
                TextureDesc.TextureType = ETextureDimension::Texture2DArray;
                if (Proxy->LightType == ELightType::LT_Directional)
                {
                    BufferDesc.Size = sizeof(FDirectionalShadowMappingBlock);
                    TextureDesc.ArraySize = CASCADED_SHADOWMAP_SPLIT_COUNT;
                }
                else if (Proxy->LightType == ELightType::LT_Point)
                {
                    BufferDesc.Size = sizeof(FPointShadowMappingBlock);
                    TextureDesc.ArraySize = 6;
                }
                else if (Proxy->LightType == ELightType::LT_Spot)
                {
                    BufferDesc.Size = sizeof(FSpotShadowMappingBlock);
                    TextureDesc.ArraySize = 1;
                }
                Resource.ShadowMapUniformBuffer = Graph.CreateBuffer("ShadowMapUniformBuffer", BufferDesc);
                for (int i = 0; i < TextureDesc.ArraySize; i++)
                {
                    RDGTextureViewDesc TextureViewDesc;
                    TextureViewDesc.Format = EPixelFormat::PF_D24S8;
                    TextureViewDesc.BaseMipLevel = 0;
                    TextureViewDesc.LevelCount = 1;
                    TextureViewDesc.BaseArrayLayer = i;
                    TextureViewDesc.LayerCount = 1;
                    TextureViewDesc.ViewType = ETextureDimension::Texture2D;
                    RDGTexture* DepthArrayView = Graph.CreateTextureView("", Resource.DepthArray, TextureViewDesc);
                    Resource.DepthViews.push_back(DepthArrayView);
                }
                LightInfo.ShadowMapResources.push_back(Resource);
            }
            Lights.push_back(LightInfo);
        }

        // Initalize views
        int NumViews = Views.size();
        ViewSceneTextures.resize(NumViews);
        ViewMeshBatches.resize(NumViews);
        ViewPDIs.resize(NumViews);
        for(int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
		    FSceneView& View = Views[ViewIndex];
            
            View.ViewUniformBuffer = Graph.CreateBuffer(fmt::format("ViewUniformBuffer {}", ViewIndex), {0, sizeof(FViewShaderParameters)});
            FViewShaderParameters ViewUniformBufferData;
            const dmat4& WorldToView = View.ViewMatrix;
            const mat4& ViewToClip = View.ProjectionMatrix;
            mat4 RelativeWorldToView = WorldToView;
            RelativeWorldToView[3][0] = 0;
            RelativeWorldToView[3][1] = 0;
            RelativeWorldToView[3][2] = 0;
            ViewUniformBufferData.RelWorldToView = RelativeWorldToView;
            ViewUniformBufferData.ViewToClip = ViewToClip;
            ViewUniformBufferData.RelWorldToClip = ViewToClip * RelativeWorldToView;
            ViewUniformBufferData.ClipToView = glm::inverse(ViewToClip);
            ViewUniformBufferData.RelClipToWorld = glm::inverse(ViewToClip * RelativeWorldToView);
            ViewUniformBufferData.AbsWorldToClip = ViewToClip * mat4(WorldToView);

            ViewUniformBufferData.CameraPosition = View.Position;
            ViewUniformBufferData.CameraDirection = View.Forward;
            ViewUniformBufferData.CameraResolution = View.ScreenResolution;
            ViewUniformBufferData.CameraNearClipDist = View.NearClipDistance;
            ViewUniformBufferData.CameraFarClipDist = View.FarClipDistance;
            ViewUniformBufferData.CameraVerticalFieldOfView = View.VerticalFieldOfView;

            for (int i = 0; i < 6; i++)
                ViewUniformBufferData.FrustumPlanes[i] = dvec4(View.ViewFrustum.Planes[i].Normal, View.ViewFrustum.Planes[i].Distance);

            View.ViewUniformBuffer->SetData(ViewUniformBufferData, 0);

            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            RDGTextureDesc Desc;
            Desc.SizeX = View.ScreenResolution.x;
            Desc.SizeY = View.ScreenResolution.y;
            Desc.SizeZ = 1;
            Desc.ArraySize = 1;
            Desc.NumMips = 1;
            Desc.TextureType = ETextureDimension::Texture2D;

            Desc.Format = EPixelFormat::PF_R16G16B16A16F;
            SceneTextures.SceneColor                    = Graph.CreateTexture(fmt::format("SceneColor {}", ViewIndex), Desc);
            SceneTextures.BaseColor                     = Graph.CreateTexture(fmt::format("BaseColor {}", ViewIndex), Desc);
            SceneTextures.RelativeWorldSpacePosition    = Graph.CreateTexture(fmt::format("RelativeWorldSpacePosition {}", ViewIndex), Desc);
            SceneTextures.WorldSpaceNormal              = Graph.CreateTexture(fmt::format("WorldSpaceNormal {}", ViewIndex), Desc);
            SceneTextures.Emissive                      = Graph.CreateTexture(fmt::format("Emissive {}", ViewIndex), Desc);

            Desc.Format = EPixelFormat::PF_D24S8;
            SceneTextures.DepthStencil                  = Graph.CreateTexture(fmt::format("DepthStencil {}", ViewIndex), Desc);

            Desc.Format = EPixelFormat::PF_R16G16F;
            SceneTextures.MetallicRoughness             = Graph.CreateTexture(fmt::format("MetallicRoughness {}", ViewIndex), Desc);

            Desc.Format = EPixelFormat::PF_R8UI;
            SceneTextures.ShadingModel                  = Graph.CreateTexture(fmt::format("ShadingModel {}", ViewIndex), Desc);

            SceneTextures.SceneColorSRV                 = Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.SceneColor));
            SceneTextures.BaseColorSRV                  = Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.BaseColor));
            SceneTextures.RelativeWorldSpacePositionSRV = Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.RelativeWorldSpacePosition));
            SceneTextures.WorldSpaceNormalSRV           = Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.WorldSpaceNormal));
            SceneTextures.EmissiveSRV                   = Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.Emissive));
            SceneTextures.DepthStencilSRV               = Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.DepthStencil));
            SceneTextures.MetallicRoughnessSRV          = Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.MetallicRoughness));
            SceneTextures.ShadingModelSRV               = Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.ShadingModel));

        }

        // Compute Visibility and collect mesh batches
        ComputeViewVisibility();
    }

    void FDeferredShadingSceneRenderer::ComputeViewVisibility()
    {
        static UTexture* IBL_BRDF_LUT = GetContentManager()->GetTextureByPath("/Textures/IBL_BRDF_LUT.nasset");
        std::vector<int> Index(Views.size(), 0);
        NILOU_LOG(Info, "Primitive count: {}", Scene->AddedPrimitiveSceneInfos.size())

        for (FPrimitiveSceneInfo* PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
        {
            if (!ViewFamily.ShowOnlyComponents.empty() && !ViewFamily.ShowOnlyComponents.contains(PrimitiveInfo->Primitive))
                continue;
            if (ViewFamily.HiddenComponents.contains(PrimitiveInfo->Primitive))
                continue;

            uint32 ViewBits = 0;
            FMeshElementCollector MeshCollector(ViewMeshBatches, ViewPDIs, PrimitiveInfo->SceneProxy);
            for(int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                FSceneView& View = Views[ViewIndex];
                // It's a very simple frustum culling...
                // TODO: BVH etc.
                bool bFrustumCulled = View.ViewFrustum.IsBoxOutSideFrustum(PrimitiveInfo->SceneProxy->GetBounds());
                if (!bFrustumCulled)
                    ViewBits |= (1 << ViewIndex);
            }

            MeshCollector.PrimitiveSceneProxy = PrimitiveInfo->SceneProxy;
            PrimitiveInfo->SceneProxy->GetDynamicMeshElements(Views, ViewBits, MeshCollector);
        }
    }

    void FDeferredShadingSceneRenderer::Render(RenderGraph& Graph)
    {
        FDynamicRHI *RHICmdList = FDynamicRHI::GetDynamicRHI();

        GetAppication()->GetPreRenderDelegate().Broadcast(RHICmdList, Scene);

        UpdateReflectionProbeFactors();

        InitViews(Graph);

        RenderPreZPass(Graph);

        RenderCSMShadowPass(Graph);

        RenderBasePass(Graph);

        RenderLightingPass(Graph);

        RenderViewElementPass(Graph);

        RenderToScreen(Graph);

        GetAppication()->GetPostRenderDelegate().Broadcast(RHICmdList, Scene);

        // ShadowMapResourcesPool.FreeAll();
        // SceneTexturesPool.FreeAll();
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

    void FDeferredShadingSceneRenderer::UpdateReflectionProbeFactors()
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
    
    void FDeferredShadingSceneRenderer::RenderToScreen(RenderGraph& Graph)
    {
        FTextureRenderTargetResource* RenderTargetResource = ViewFamily.Viewport.RenderTarget;
        RHIRenderTargetLayout RTLayout;
        RTLayout.NumRenderTargetsEnabled = 1;
        RTLayout.RenderTargetFormats[0] = RenderTargetResource->GetTextureRDG()->Desc.Format;

        // default sampler state of this pass
        RHISamplerState* SamplerStateRHI = TStaticSamplerState<>::GetRHI();

        // construct PSO initializer and create PSO
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
        PSOInitializer.RTLayout = RTLayout;
        FRHIPipelineState *PSO = RHICreateGraphicsPipelineState(PSOInitializer);

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneTextures SceneTextures = ViewSceneTextures[ViewIndex];

            RDGDescriptorSet* DescriptorSetPS = Graph.CreateDescriptorSet<FRenderToScreenPixelShader>(0, 0);
            RDGBuffer* UniformBlock = Graph.CreateUniformBuffer<FRenderToScreenPixelShader::UniformBlock>("FRenderToScreenPixelShader UniformBlock");
            auto UniformBlockData = UniformBlock->GetData<FRenderToScreenPixelShader::UniformBlock>();
            if (ViewFamily.bIsSceneCapture)
            {
                if (ViewFamily.CaptureSource == SCS_SceneDepth)
                {
                    UniformBlockData->GammaCorrection = 1.f;
                    UniformBlockData->bEnableToneMapping = 0;
                    DescriptorSetPS->SetSampler(
                        "SceneColor", 
                        Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.DepthStencil)), SamplerStateRHI);
                }
                else if (ViewFamily.CaptureSource == SCS_LinearColor)
                {
                    UniformBlockData->GammaCorrection = 1.f;
                    UniformBlockData->bEnableToneMapping = 0;
                    DescriptorSetPS->SetSampler(
                        "SceneColor", 
                        Graph.CreateSRV(RDGTextureSRVDesc(SceneTextures.SceneColor)), SamplerStateRHI);
                }
                else if (ViewFamily.CaptureSource == SCS_GammaColor)
                {
                    UniformBlockData->GammaCorrection = ViewFamily.GammaCorrection;
                    UniformBlockData->bEnableToneMapping = 1;
                    DescriptorSetPS->SetSampler(
                        "SceneColor", SamplerStateRHI, 
                        SceneTextures.SceneColor);
                }
                else 
                {
                    Ncheckf(false, "Unknown scene capture source");
                }
            }
            else 
            {
                UniformBlockData->GammaCorrection = ViewFamily.GammaCorrection;
                UniformBlockData->bEnableToneMapping = ViewFamily.bEnableToneMapping;
                DescriptorSetPS->SetSampler(
                    "SceneColor", SamplerStateRHI, 
                    SceneTextures.SceneColor);
            }
            DescriptorSetPS->SetUniformBuffer("PIXEL_UNIFORM_BLOCK", UniformBlock);
            RDGGraphicsPassDesc PassDesc{};
            PassDesc.DescriptorSets = { DescriptorSetPS };
            PassDesc.RenderTargets = RenderTargets;
            Graph.AddGraphicsPass(
                PassDesc,
                [=](RHICommandList& RHICmdList)
                {
                    // FRHIRenderPassInfo PassInfo(OutputRenderTarget, ViewInfo.ScreenResolution, true, true, true);
                    // RHICmdList->RHIBeginRenderPass(PassInfo);
                    {
                        RHIGetError();
                        RHICmdList.BindPipeline(PSO, EPipelineBindPoint::Graphics);
                        RHIGetError();

                        RHICmdList.BindVertexBuffer(0, PositionVertexBuffer.VertexBufferRDG->GetRHI(), 0);
                        RHICmdList.BindVertexBuffer(1, UVVertexBuffer.VertexBufferRDG->GetRHI(), 0);

                        RHICmdList.BindDescriptorSets(
                            PSO->GetPipelineLayout(), 
                            { {0, DescriptorSetPS->GetRHI()} }, 
                            EPipelineBindPoint::Graphics);

                        RHICmdList.DrawArrays(4, 1, 0, 0);
                    }
                    // RHICmdList.RHIEndRenderPass();
                }
            );
        }


        
    }

    // FSceneTextures* FDeferredShadingSceneRenderer::CreateSceneTextures(const SceneTextureCreateInfo &CreateInfo)
    // {
    //     return new FSceneTexturesDeferred(CreateInfo);
    // }

    // FShadowMapResources* FDeferredShadingSceneRenderer::CreateLightRenderResources(const ShadowMapResourcesCreateInfo &CreateInfo)
    // {
    //     return new FShadowMapResources(CreateInfo);
    // }
}