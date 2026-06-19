// #include <glm/gtc/matrix_transform.hpp>
#include "Engine/TextureRenderTarget.h"
#include "Logging/LogMacros.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

#include "Renderer/DeferredShadingSceneRenderer.h"
// #include "Components/LightSceneProxy.h"
// #include "Components/PrimitiveSceneProxy.h"
#include "DynamicRHI.h"
#include "Frustum.h"
#include "Materials/Material.h"
#include "ShaderMap.h"
#include "RenderGraphUtils.h"

#include "Renderer/ShadowDepthPassRendering.h"
#include "Renderer/BasePassRendering.h"
#include "Renderer/LightingPassRendering.h"

#include "GameFramework/ReflectionProbe.h"
#include "RenderToScreenPixelShader.generated.h"

#ifdef NILOU_DEBUG
// #include "CoordinateAxis.h"
#endif


namespace nilou {

    static FMatrix44f MakeScreenToClipMatrix(const FMatrix44f& ProjectionMatrix, ECameraProjectionMode ProjectionMode)
    {
        FMatrix44f Result(1.0f);
        const bool bPerspectiveProjection = ProjectionMode == ECameraProjectionMode::Perspective;
        Result[0] = FVector4f(1.0f, 0.0f, 0.0f, 0.0f);
        Result[1] = FVector4f(0.0f, 1.0f, 0.0f, 0.0f);
        Result[2] = FVector4f(0.0f, 0.0f, ProjectionMatrix[2][2], bPerspectiveProjection ? 1.0f : 0.0f);
        Result[3] = FVector4f(0.0f, 0.0f, ProjectionMatrix[2][3], bPerspectiveProjection ? 0.0f : 1.0f);
        return Result;
    }

    IMPLEMENT_SHADER_TYPE(FScreenQuadVertexShader, "/Shaders/Private/GlobalShaders/ScreenQuadVertexShader.slang", "Main", EShaderFrequency::Vertex)
    IMPLEMENT_SHADER_TYPE(FRenderToScreenPixelShader, "/Shaders/Private/GlobalShaders/RenderToScreenPixelShader.slang", "Main", EShaderFrequency::Pixel)
    DEFINE_GRAPHICS_PIPELINE(FRenderToScreenPipeline, FScreenQuadVertexShader, FRenderToScreenPixelShader)

    FDeferredShadingSceneRenderer *Renderer = nullptr;

    FSceneRenderer *FSceneRenderer::CreateSceneRenderer(const FSceneViewFamily& ViewFamily)
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

    FSceneRenderer::FSceneRenderer(const FSceneViewFamily& InViewFamily)
        : Scene(InViewFamily.Scene)
        , ViewFamily(InViewFamily)
        , Views(ViewFamily.Views)
    {

    }

    FDeferredShadingSceneRenderer::FDeferredShadingSceneRenderer(const FSceneViewFamily& ViewFamily)
        : FSceneRenderer(ViewFamily)
    {

    }


    void FDeferredShadingSceneRenderer::InitViews(RenderGraph& Graph)
    {
        // Initialize lights
        // Lights are relavant to views, for example directional lights.
        VisibleLightInfos.reserve(Scene->AddedLightSceneInfos.size());
        for (FLightSceneInfo* LightSceneInfo : Scene->AddedLightSceneInfos)
        {
            FLightSceneProxy* Proxy = LightSceneInfo->SceneProxy;
            FVisibleLightInfo LightInfo;
            LightInfo.LightSceneProxy = Proxy;
            int NumRelevantViews = 1;
            if (Proxy->LightType == ELightType::Directional)
                NumRelevantViews = Views.size();
            for (int ViewIndex = 0; ViewIndex < NumRelevantViews; ViewIndex++)
            {
                FShadowMapResource Resource;
                RDGTextureDesc TextureDesc;
                TextureDesc.SizeX = 1024;
                TextureDesc.SizeY = 1024;
                TextureDesc.NumMips = 1;
                TextureDesc.Format = EPixelFormat::PF_D32FS8;
                TextureDesc.TextureType = ETextureDimension::Texture2DArray;
                int32 BufferSize = 0;
                constexpr int CASCADED_SHADOWMAP_SPLIT_COUNT = 8;
                if (Proxy->LightType == ELightType::Directional)
                {
                    BufferSize = sizeof(shader::ShadowMappingFrustum<Std430Layout>) * CASCADED_SHADOWMAP_SPLIT_COUNT;
                    TextureDesc.ArraySize = CASCADED_SHADOWMAP_SPLIT_COUNT;
                }
                // else if (Proxy->LightType == ELightType::Point)
                // {
                //     BufferSize = sizeof(shader::ShadowMappingFrustum<Std430Layout>) * 6;
                //     TextureDesc.ArraySize = 6;
                // }
                // else if (Proxy->LightType == ELightType::Spot)
                // {
                //     BufferSize = sizeof(shader::ShadowMappingFrustum<Std430Layout>);
                //     TextureDesc.ArraySize = 1;
                // }
                else 
                {
                    Ncheck(false);  // not supported yet
                }
                Resource.DepthArray = Graph.CreateTexture(NFormat("Shadow DepthArray of View{}", ViewIndex), TextureDesc);
                Resource.ShadowMappingParameters = Graph.CreateParameterBlock<shader::ShadowMappingParameters>("ShadowMappingParameters");
                Resource.ShadowMappingParameters->frustums = Graph.CreateBuffer("ShadowMap frustums", RDGBufferDesc(BufferSize, sizeof(shader::ShadowMappingFrustum<Std430Layout>), EBufferUsageFlags::StorageBuffer));
                Resource.ShadowMappingParameters->shadowMaps = { Resource.DepthArray->GetDefaultView() };
                for (int i = 0; i < TextureDesc.ArraySize; i++)
                {
                    RDGTextureViewDesc TextureViewDesc;
                    TextureViewDesc.Format = EPixelFormat::PF_D32FS8;
                    TextureViewDesc.SubresourceRange = RHITextureSubresourceRange::Make(0, 1, i, 1, 0, 2);
                    TextureViewDesc.ViewType = ETextureDimension::Texture2D;
                    RDGTextureView* DepthArrayView = Graph.CreateTextureView(NFormat("DepthArrayView {}", i), Resource.DepthArray, TextureViewDesc);
                    Resource.DepthViews.push_back(DepthArrayView);
                }
                Graph.UpdateParameterBlock(Resource.ShadowMappingParameters);
                LightInfo.ShadowMapResources.push_back(Resource);
            }
            VisibleLightInfos.push_back(LightInfo);
        }

        // Initalize views
        int NumViews = Views.size();
        ViewSceneTextures.resize(NumViews);
        ViewMeshBatches.resize(NumViews);
        ViewPDIs.resize(NumViews);
        for(int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
		    FSceneView& View = Views[ViewIndex];
            
            View.ViewUniformBuffer = Graph.CreateParameterBlock<shader::FViewShaderParameters>(NFormat("ViewUniformBuffer {}", ViewIndex));
            auto ViewUniformBuffer = View.ViewUniformBuffer;
            const FMatrix& WorldToView = View.ViewMatrix;
            const FMatrix44f& ViewToClip = View.ProjectionMatrix;
            FMatrix44f RelativeWorldToView = WorldToView;
            RelativeWorldToView[3][0] = 0;
            RelativeWorldToView[3][1] = 0;
            RelativeWorldToView[3][2] = 0;
            ViewUniformBuffer->RelWorldToView = RelativeWorldToView;
            ViewUniformBuffer->ViewToClip = ViewToClip;
            ViewUniformBuffer->RelWorldToClip = ViewToClip * RelativeWorldToView;
            ViewUniformBuffer->ClipToView = glm::inverse(ViewToClip);
            const FMatrix44f RelClipToWorld = glm::inverse(ViewToClip * RelativeWorldToView);
            ViewUniformBuffer->RelClipToWorld = RelClipToWorld;
            const FMatrix44f ScreenToClip = MakeScreenToClipMatrix(ViewToClip, View.ProjectionMode);
            ViewUniformBuffer->ScreenToRelativeWorld = RelClipToWorld * ScreenToClip;
            ViewUniformBuffer->AbsWorldToClip = ViewToClip * FMatrix44f(WorldToView);
            ViewUniformBuffer->bIsOrthoProjection =
                View.ProjectionMode == ECameraProjectionMode::Orthographic ? 1u : 0u;
            ViewUniformBuffer->ViewRectMin = FVector2f(0.0f, 0.0f);
            const float ViewWidth = static_cast<float>(View.ScreenResolution.x);
            const float ViewHeight = static_cast<float>(View.ScreenResolution.y);
            ViewUniformBuffer->ViewSizeAndInvSize = FVector4f(
                ViewWidth, ViewHeight, 1.0f / ViewWidth, 1.0f / ViewHeight);

            ViewUniformBuffer->CameraPosition = View.Position;
            ViewUniformBuffer->CameraDirection = View.Forward;
            ViewUniformBuffer->CameraResolution = View.ScreenResolution;
            ViewUniformBuffer->CameraNearClipDist = View.NearClipDistance;
            ViewUniformBuffer->CameraFarClipDist = View.FarClipDistance;
            ViewUniformBuffer->CameraVerticalFieldOfView = View.VerticalFieldOfView;

            for (int i = 0; i < 6; i++)
                ViewUniformBuffer->FrustumPlanes[i] = FVector4f(View.ViewFrustum.Planes[i].Normal, View.ViewFrustum.Planes[i].Distance);

            Graph.UpdateParameterBlock(ViewUniformBuffer);

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

            Desc.Format = EPixelFormat::PF_D32FS8;
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
        // NILOU_LOG(Display, "Primitive count: {}", Scene->AddedPrimitiveSceneInfos.size())

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

        UpdateReflectionProbeFactors();

        InitViews(Graph);

        RenderPrePass(Graph);

        RenderCSMShadowPass(Graph);

        RenderBasePass(Graph);

        RenderLightingPass(Graph);

        RenderSkyAtmospherePass(Graph);

        RenderViewElementPass(Graph);

        RenderToScreen(Graph);

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
                FVector Min = ReflectionProbe->SceneProxy->Location - ReflectionProbe->SceneProxy->Extent/2.0;
                FVector Max = ReflectionProbe->SceneProxy->Location + ReflectionProbe->SceneProxy->Extent/2.0;
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

        // construct PSO initializer and create PSO (use compiled global graphics pipeline for Slang + SPIR-V pair)
        RHIGraphicsPipelineShaders* RenderToScreenPipeline = GetGlobalGraphicsPipeline<FRenderToScreenPipeline>();
        FGraphicsPipelineStateInitializer PSOInitializer;
        PSOInitializer.Shaders = *RenderToScreenPipeline;
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

            auto Parameters = Graph.CreateParameterBlock<shader::FRenderToScreenParameters>(NFormat("shader::FRenderToScreenParameters {}", ViewIndex));

            if (ViewFamily.bIsSceneCapture)
            {
                if (ViewFamily.CaptureSource == SCS_LinearColor)
                {
                    Parameters->GammaCorrection = 1.f;
                    Parameters->bEnableToneMapping = 0;
                    Parameters->SceneColor = SceneTextures.SceneColor->GetDefaultView();
                    Parameters->SceneColorSampler = SamplerStateRHI;
                }
                else if (ViewFamily.CaptureSource == SCS_GammaColor)
                {
                    Parameters->GammaCorrection = ViewFamily.GammaCorrection;
                    Parameters->bEnableToneMapping = 1;
                    Parameters->SceneColor = SceneTextures.SceneColor->GetDefaultView();
                    Parameters->SceneColorSampler = SamplerStateRHI;
                }
                else 
                {
                    Ncheckf(false, "Unknown scene capture source");
                }
            }
            else 
            {
                Parameters->GammaCorrection = ViewFamily.GammaCorrection;
                Parameters->bEnableToneMapping = ViewFamily.bEnableToneMapping;
                Parameters->SceneColor = SceneTextures.SceneColor->GetDefaultView();
                Parameters->SceneColorSampler = SamplerStateRHI;
            }
            Graph.UpdateParameterBlock(Parameters);

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
                [=](FRDGPass* Pass)
                {
                    Pass->AddParameterBlock(Parameters);
                },
                [=](RHICommandList& RHICmdList)
                {
                    // FRHIRenderPassInfo PassInfo(OutputRenderTarget, ViewInfo.ScreenResolution, true, true, true);
                    // RHICmdList->RHIBeginRenderPass(PassInfo);
                    {
                        RHICmdList.BindGraphicsPipelineState(PSO);

                        RHICmdList.BindVertexBuffer(0, ScreenQuadVertexBuffer->GetRHI(), 0);
                        RHICmdList.BindIndexBuffer(ScreenQuadIndexBuffer->GetRHI(), 0);

                        int32 DescriptorSetIndex = PSO->GetPipelineLayout()->GetSetIndex("Params");
                        RHICmdList.BindDescriptorSets(
                            PSO->GetPipelineLayout(), 
                            { {DescriptorSetIndex, Parameters->GetDescriptorSet()->GetRHI()} }, 
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