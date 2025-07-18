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
#include "RenderGraphUtils.h"

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

    template<typename T>
    void AddUnique(std::vector<T>& Vector, const T& Value)
    {
        if (std::find(Vector.begin(), Vector.end(), Value) == Vector.end())
        {
            Vector.push_back(Value);
        }
    }

    std::vector<RDGBuffer*> FParallelMeshDrawCommands::GetVertexBuffers() const
    {
        std::vector<RDGBuffer*> VertexBuffers;
        for (int i = 0; i < MeshCommands.size(); i++)
        {
            const FMeshDrawCommand& MeshDrawCommand = MeshCommands[i];
            for (int j = 0; j < MeshDrawCommand.VertexStreams.size(); j++)
            {
                const FVertexInputStream& VertexStream = MeshDrawCommand.VertexStreams[j];
                AddUnique(VertexBuffers, VertexStream.VertexBuffer);
            }
        }
        return VertexBuffers;
    }

    std::vector<RDGBuffer*> FParallelMeshDrawCommands::GetIndexBuffers() const
    {
        std::vector<RDGBuffer*> IndexBuffers;
        for (int i = 0; i < MeshCommands.size(); i++)
        {
            const FMeshDrawCommand& MeshDrawCommand = MeshCommands[i];
            AddUnique(IndexBuffers, MeshDrawCommand.IndexBuffer);
        }
        return IndexBuffers;
    }

    std::vector<RDGDescriptorSet*> FParallelMeshDrawCommands::GetDescriptorSets() const
    {
        std::vector<RDGDescriptorSet*> DescriptorSets;
        for (int i = 0; i < MeshCommands.size(); i++)
        {
            const FMeshDrawCommand& MeshDrawCommand = MeshCommands[i];
            for (auto& [SetIndex, DescriptorSet] : MeshDrawCommand.DescriptorSets)
            {
                AddUnique(DescriptorSets, DescriptorSet);
            }
        }
        return DescriptorSets;
    }

    FSceneRenderer::FSceneRenderer(FSceneViewFamily& InViewFamily)
        : Scene(InViewFamily.Scene)
        , ViewFamily(InViewFamily)
        , Views(InViewFamily.Views)
    {

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
            LightInfo.LightSceneProxy = Proxy;
            LightInfo.LightType = Proxy->LightType;
            LightInfo.LightUniformBuffer = LightSceneInfo->LightUniformBuffer.GetReference();
            int NumRelevantViews = 1;
            if (Proxy->LightType == ELightType::LT_Directional)
                NumRelevantViews = Views.size();
            for (int ViewIndex = 0; ViewIndex < NumRelevantViews; ViewIndex++)
            {
                FShadowMapResource Resource;
                RDGTextureDesc TextureDesc;
                TextureDesc.SizeX = 1024;
                TextureDesc.SizeY = 1024;
                TextureDesc.NumMips = 1;
                TextureDesc.Format = EPixelFormat::PF_D24S8;
                TextureDesc.TextureType = ETextureDimension::Texture2DArray;
                int32 BufferSize = 0;
                if (Proxy->LightType == ELightType::LT_Directional)
                {
                    BufferSize = sizeof(FDirectionalShadowMappingBlock);
                    TextureDesc.ArraySize = CASCADED_SHADOWMAP_SPLIT_COUNT;
                }
                else if (Proxy->LightType == ELightType::LT_Point)
                {
                    BufferSize = sizeof(FPointShadowMappingBlock);
                    TextureDesc.ArraySize = 6;
                }
                else if (Proxy->LightType == ELightType::LT_Spot)
                {
                    BufferSize = sizeof(FSpotShadowMappingBlock);
                    TextureDesc.ArraySize = 1;
                }
                Resource.Frustums.resize(TextureDesc.ArraySize);
                Resource.ShadowMapUniformBuffer = Graph.CreateBuffer("ShadowMapUniformBuffer", RDGBufferDesc(BufferSize, EBufferUsageFlags::UniformBuffer));
                Resource.DepthArray = Graph.CreateTexture(NFormat("Shadow DepthArray of View{}", ViewIndex), TextureDesc);
                for (int i = 0; i < TextureDesc.ArraySize; i++)
                {
                    RDGTextureViewDesc TextureViewDesc;
                    TextureViewDesc.Format = EPixelFormat::PF_D24S8;
                    TextureViewDesc.BaseMipLevel = 0;
                    TextureViewDesc.LevelCount = 1;
                    TextureViewDesc.BaseArrayLayer = i;
                    TextureViewDesc.LayerCount = 1;
                    TextureViewDesc.ViewType = ETextureDimension::Texture2D;
                    RDGTextureView* DepthArrayView = Graph.CreateTextureView(NFormat("DepthArrayView {}", i), Resource.DepthArray, TextureViewDesc);
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
            
            View.ViewUniformBuffer = Graph.CreateUniformBuffer<FViewShaderParameters>(NFormat("ViewUniformBuffer {}", ViewIndex));
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

            Graph.QueueBufferUpload(View.ViewUniformBuffer, &ViewUniformBufferData, sizeof(ViewUniformBufferData));

            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            RDGTextureDesc Desc;
            Desc.SizeX = View.ScreenResolution.x;
            Desc.SizeY = View.ScreenResolution.y;
            Desc.SizeZ = 1;
            Desc.ArraySize = 1;
            Desc.NumMips = 1;
            Desc.TextureType = ETextureDimension::Texture2D;

            Desc.Format = EPixelFormat::PF_R16G16B16A16F;
            SceneTextures.SceneColor                    = Graph.CreateTexture(NFormat("SceneColor {}", ViewIndex), Desc);
            SceneTextures.BaseColor                     = Graph.CreateTexture(NFormat("BaseColor {}", ViewIndex), Desc);
            SceneTextures.RelativeWorldSpacePosition    = Graph.CreateTexture(NFormat("RelativeWorldSpacePosition {}", ViewIndex), Desc);
            SceneTextures.WorldSpaceNormal              = Graph.CreateTexture(NFormat("WorldSpaceNormal {}", ViewIndex), Desc);
            SceneTextures.Emissive                      = Graph.CreateTexture(NFormat("Emissive {}", ViewIndex), Desc);

            Desc.Format = EPixelFormat::PF_D24S8;
            SceneTextures.DepthStencil                  = Graph.CreateTexture(NFormat("DepthStencil {}", ViewIndex), Desc);

            Desc.Format = EPixelFormat::PF_R16G16F;
            SceneTextures.MetallicRoughness             = Graph.CreateTexture(NFormat("MetallicRoughness {}", ViewIndex), Desc);

            Desc.Format = EPixelFormat::PF_R8UI;
            SceneTextures.ShadingModel                  = Graph.CreateTexture(NFormat("ShadingModel {}", ViewIndex), Desc);

        }

        // Compute Visibility and collect mesh batches
        ComputeViewVisibility(ViewFamily, ViewMeshBatches, ViewPDIs);
    }

    void FDeferredShadingSceneRenderer::ComputeViewVisibility(
        FSceneViewFamily& ViewFamily, 
        std::vector<std::vector<FMeshBatch>>& OutViewMeshBatches, 
        std::vector<FViewElementPDI>& OutViewPDIs)
    {
        std::vector<FSceneView>& Views = ViewFamily.Views;
        OutViewMeshBatches.resize(Views.size());
        OutViewPDIs.resize(Views.size());
        NILOU_LOG(Display, "Primitive count: {}", Scene->AddedPrimitiveSceneInfos.size())

        for (FPrimitiveSceneInfo* PrimitiveInfo : Scene->AddedPrimitiveSceneInfos)
        {
            if (!ViewFamily.ShowOnlyComponents.empty() && !ViewFamily.ShowOnlyComponents.contains(PrimitiveInfo->Primitive))
                continue;
            if (ViewFamily.HiddenComponents.contains(PrimitiveInfo->Primitive))
                continue;

            uint32 ViewBits = 0;
            FMeshElementCollector MeshCollector(OutViewMeshBatches, OutViewPDIs, PrimitiveInfo->SceneProxy);
            for(int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
            {
                FSceneView& View = Views[ViewIndex];
                // It's a very simple frustum culling...
                // TODO: BVH etc.
                bool bFrustumCulled = View.ViewFrustum.IsBoxOutSideFrustum(PrimitiveInfo->SceneProxy->GetBounds());
                if (!bFrustumCulled)
                    ViewBits |= (1 << ViewIndex);
            }
            PrimitiveInfo->SceneProxy->GetDynamicMeshElements(Views, ViewBits, MeshCollector);
        }
    }

    void FDeferredShadingSceneRenderer::Render(RenderGraph& Graph)
    {
        FDynamicRHI *RHICmdList = FDynamicRHI::Get();

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

    float IntersectVolume(const FBoxSphereBounds& Bounds1, const FBoxSphereBounds& Bounds2)
    {
        FBox Box1 = Bounds1.GetBox();
        FBox Box2 = Bounds2.GetBox();
        double xIntersection = std::max(0.0, std::min(Box1.Max.x, Box2.Max.x) - std::max(Box1.Min.x, Box2.Min.x));

        if (xIntersection <= 0.0)
            return 0.0;
        

        double yIntersection = std::max(0.0, std::min(Box1.Max.y, Box2.Max.y) - std::max(Box1.Min.y, Box2.Min.y));

        if (yIntersection <= 0.0)
            return 0.0;
        

        double zIntersection = std::max(0.0, std::min(Box1.Max.z, Box2.Max.z) - std::max(Box1.Min.z, Box2.Min.z));

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
            FBoxSphereBounds PrimitiveExtent = Primitive->SceneProxy->Bounds;
            float total_volume = 0;
            for (auto ReflectionProbe : Scene->ReflectionProbes)
            {
                dvec3 Min = ReflectionProbe->SceneProxy->Location - ReflectionProbe->SceneProxy->Extent/2.0;
                dvec3 Max = ReflectionProbe->SceneProxy->Location + ReflectionProbe->SceneProxy->Extent/2.0;
                FBoxSphereBounds ReflectionProbeExtent(FBox(Min, Max));
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
        RDGTexture* RenderTarget = nullptr;
        if (RenderTargetResource)
        {
            RenderTarget = RenderTargetResource->GetTextureRDG();
        }
        else
        {
            RenderTarget = Graph.GetSwapChainTexture();
        }
        RHIRenderTargetLayout RTLayout;
        RTLayout.ColorAttachments[0].Format = RenderTarget->Desc.Format;

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
        PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
        PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
        PSOInitializer.BlendState = TStaticBlendState<>::GetRHI();
        PSOInitializer.VertexDeclaration = RDGGetScreenQuadVertexDeclaration();
        PSOInitializer.RTLayout = RTLayout;
        RHIGraphicsPipelineState *PSO = RHICreateGraphicsPipelineState(PSOInitializer);

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneTextures SceneTextures = ViewSceneTextures[ViewIndex];

            RDGDescriptorSet* DescriptorSetPS = Graph.CreateDescriptorSet("RenderToScreenPS DescriptorSet", RenderToScreenPS->GetDescriptorSetLayout(0));
            FRenderToScreenParameters Parameters;
            if (ViewFamily.bIsSceneCapture)
            {
                if (ViewFamily.CaptureSource == SCS_SceneDepth)
                {
                    Parameters.GammaCorrection = 1.f;
                    Parameters.bEnableToneMapping = 0;
                    DescriptorSetPS->SetSampler(
                        "SceneColor", 
                        SceneTextures.DepthStencil->GetDefaultView(), SamplerStateRHI);
                }
                else if (ViewFamily.CaptureSource == SCS_LinearColor)
                {
                    Parameters.GammaCorrection = 1.f;
                    Parameters.bEnableToneMapping = 0;
                    DescriptorSetPS->SetSampler(
                        "SceneColor", 
                        SceneTextures.SceneColor->GetDefaultView(), SamplerStateRHI);
                }
                else if (ViewFamily.CaptureSource == SCS_GammaColor)
                {
                    Parameters.GammaCorrection = ViewFamily.GammaCorrection;
                    Parameters.bEnableToneMapping = 1;
                    DescriptorSetPS->SetSampler(
                        "SceneColor", 
                        SceneTextures.SceneColor->GetDefaultView(), SamplerStateRHI);
                }
                else 
                {
                    Ncheckf(false, "Unknown scene capture source");
                }
            }
            else 
            {
                Parameters.GammaCorrection = ViewFamily.GammaCorrection;
                Parameters.bEnableToneMapping = ViewFamily.bEnableToneMapping;
                DescriptorSetPS->SetSampler(
                    "SceneColor", 
                    SceneTextures.SceneColor->GetDefaultView(), SamplerStateRHI);
            }
            auto UniformBuffer = Graph.CreateUniformBuffer<FRenderToScreenParameters>(NFormat("FRenderToScreenParameters {}", ViewIndex));
            Graph.QueueBufferUpload(UniformBuffer, &Parameters, sizeof(Parameters));
            DescriptorSetPS->SetUniformBuffer("PIXEL_UNIFORM_BLOCK", UniformBuffer);

            RDGBuffer* ScreenQuadVertexBuffer = RDGGetScreenQuadVertexBuffer(Graph);
            RDGBuffer* ScreenQuadIndexBuffer = RDGGetScreenQuadIndexBuffer(Graph);

            RDGRenderTargets RenderTargets;
            RenderTargets.ColorAttachments[0] = RenderTarget->GetDefaultView();
            RDGPassDesc PassDesc{NFormat("RenderToScreen {}", ViewIndex)};
            PassDesc.bNeverCull = true;
            Graph.AddGraphicsPass(
                PassDesc,
                RenderTargets,
                { ScreenQuadIndexBuffer },
                { ScreenQuadVertexBuffer },
                { DescriptorSetPS },
                [=](RHICommandList& RHICmdList)
                {
                    // FRHIRenderPassInfo PassInfo(OutputRenderTarget, ViewInfo.ScreenResolution, true, true, true);
                    // RHICmdList->RHIBeginRenderPass(PassInfo);
                    {
                        RHIGetError();
                        RHICmdList.BindGraphicsPipelineState(PSO);
                        RHIGetError();

                        RHICmdList.BindVertexBuffer(0, ScreenQuadVertexBuffer->GetRHI(), 0);
                        RHICmdList.BindIndexBuffer(ScreenQuadIndexBuffer->GetRHI(), 0);

                        RHICmdList.BindDescriptorSets(
                            PSO->GetPipelineLayout(), 
                            { {0, DescriptorSetPS->GetRHI()} }, 
                            EPipelineBindPoint::Graphics);

                        RHICmdList.DrawIndexed(6, 1, 0, 0, 0);
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