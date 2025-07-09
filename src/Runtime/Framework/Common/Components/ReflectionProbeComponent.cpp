#include "ReflectionProbeComponent.h"
#include "Shader.h"
#include "TextureCube.h"
#include "Common/World.h"


namespace nilou {
    DECLARE_GLOBAL_SHADER(FBrdfLUTShader);
    IMPLEMENT_SHADER_TYPE(FBrdfLUTShader, "/Shaders/GlobalShaders/BrdfLUT.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FIrradianceEnvTextureShader);
    IMPLEMENT_SHADER_TYPE(FIrradianceEnvTextureShader, "/Shaders/GlobalShaders/IrradianceEnvTextureShader.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FPrefilteredEnvTextureShader);
    IMPLEMENT_SHADER_TYPE(FPrefilteredEnvTextureShader, "/Shaders/GlobalShaders/PrefilteredEnvTextureShader.comp", EShaderFrequency::SF_Compute, Global)



    void UReflectionProbeComponent::UpdateSceneCaptureContents(FScene* Scene)
    {
        UpdateSceneCaptureContents_Internal(Scene, GetComponentLocation()+OriginOffset);
        ENQUEUE_RENDER_COMMAND(UReflectionProbeComponent_UpdateSceneCaptureContents)(
            [Scene, this](RenderGraph&) 
            {
                UpdateSceneCaptureContents_RenderThread(Scene);
            });
    }

    void UReflectionProbeComponent::OnRegister()
    {
        USceneCaptureComponentCube::OnRegister();
        
        ENQUEUE_RENDER_COMMAND(UReflectionProbeComponent_OnRegister)(
            [this](RenderGraph& Graph) 
            {
                IrradianceShaderUniformBuffer = RenderGraph::CreatePooledUniformBuffer<IrradianceEnvTextureShaderBlock>("IrradianceEnvTextureShaderBlock", nullptr);
                PrefilterShaderUniformBuffer = RenderGraph::CreatePooledUniformBuffer<PrefilteredEnvTextureShaderBlock>("PrefilteredEnvTextureShaderBlock", nullptr);
            });

        if (WorldPrivate) 
        {
            if (WorldPrivate->Scene)
            {
                WorldPrivate->Scene->AddReflectionProbe(this);
            }
        }

        
        // UTexture2D* lut = GetContentManager()->CreateAsset<UTexture2D>("/Textures/my_lut.nasset");
        // lut->Name = "IBL_BRDF_LUT";
        // lut->ImageData = std::make_shared<FImage2D>(512, 512, EPixelFormat::PF_R16G16F);
        // lut->TextureParams.Wrap_R = ETextureWrapModes::TW_Clamp;
        // lut->TextureParams.Wrap_S = ETextureWrapModes::TW_Clamp;
        // lut->TextureParams.Wrap_T = ETextureWrapModes::TW_Clamp;
        // lut->TextureParams.Min_Filter = ETextureFilters::TF_Linear;
        // lut->TextureParams.Mag_Filter = ETextureFilters::TF_Linear;
        // lut->UpdateResource();
    }

    void UReflectionProbeComponent::OnUnregister()
    {
        
        ENQUEUE_RENDER_COMMAND(UReflectionProbeComponent_OnRegister)(
            [this](RenderGraph& Graph) 
            {
                IrradianceShaderUniformBuffer = nullptr;
                PrefilterShaderUniformBuffer = nullptr;
            });

        if (WorldPrivate) 
        {
            if (WorldPrivate->Scene)
            {
                WorldPrivate->Scene->RemoveReflectionProbe(this);
            }
        }

        USceneCaptureComponentCube::OnUnregister();
    }

    void UReflectionProbeComponent::UpdateSceneCaptureContents_RenderThread(FScene* Scene)
    {
#if NILOU_ENABLE_REFLECTION_PROBE
        // UTexture2D* lut = (UTexture2D*)GetContentManager()->GetTextureByPath("/Textures/my_lut.nasset");
        // FShaderPermutationParameters PermutationParameters(&FBrdfLUTShader::StaticType, 0);
        // FShaderInstance *BrdfLUTShader = GetGlobalShader(PermutationParameters);
        // FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(BrdfLUTShader);

        // RHICmdList->RHISetShaderImage(
        //     PSO, EPipelineStage::PS_Compute,
        //     "LUT", lut->GetResource()->TextureRHI.get(), EDataAccessFlag::DA_WriteOnly);
        
        // RHICmdList->RHIDispatch(16, 16, 1);
        // lut->ReadPixelsRenderThread(RHICmdList);

        {
            
            IrradianceShaderUniformBuffer->Data.TextureSize = IrradianceTexture->GetSizeX();
            IrradianceShaderUniformBuffer->UpdateUniformBuffer();

            FShaderPermutationParameters PermutationParameters(&FIrradianceEnvTextureShader::StaticType, 0);
            FShaderInstance *IrradianceShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(IrradianceShader->GetComputeShaderRHI());

            RHICmdList->RHISetShaderSampler(
                PSO, EPipelineStage::PS_Compute,
                "EnvironmentTexture", *TextureTarget->GetResource()->GetSamplerRHI());

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute,
                "IrradianceTexture", IrradianceTexture->GetResource()->TextureRHI.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHISetShaderUniformBuffer(
                    PSO, EPipelineStage::PS_Compute, 
                    "IrradianceEnvTextureShaderBlock", IrradianceShaderUniformBuffer->GetRHI());
            
            RHICmdList->RHIDispatch(16, 16, 1);
        }
        
        {
            FShaderPermutationParameters PermutationParameters(&FPrefilteredEnvTextureShader::StaticType, 0);
            FShaderInstance *PrefilterShader = GetGlobalShader(PermutationParameters);

            int NumMips = PrefilteredTexture->NumMips;
            float delta_roughness = 1.0 / glm::max(NumMips-1, 1);
            if (PrefilteredTextureMips.size() != NumMips)
            {
                PrefilteredTextureMips.resize(NumMips);
                for (int MipIndex = 0; MipIndex < NumMips; MipIndex++)
                {
                    RHITexture* CubeMap = PrefilteredTexture->GetResource()->TextureRHI.get();
                    RHITextureCubeRef TextureView = RHICmdList->RHICreateTextureViewCube(
                        CubeMap, CubeMap->GetFormat(), 
                        MipIndex, 1);
                    PrefilteredTextureMips[MipIndex] = TextureView;
                }
            }
            for (int MipIndex = 0; MipIndex < NumMips; MipIndex++)
            {
                FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(PrefilterShader->GetComputeShaderRHI());

                RHICmdList->RHISetShaderSampler(
                    PSO, EPipelineStage::PS_Compute,
                    "EnvironmentTexture", *TextureTarget->GetResource()->GetSamplerRHI());
                PrefilterShaderUniformBuffer->Data.TextureSize = TextureTarget->GetSizeX() >> MipIndex;
                PrefilterShaderUniformBuffer->Data.roughness = MipIndex * delta_roughness;
                PrefilterShaderUniformBuffer->UpdateUniformBuffer();
                RHICmdList->RHISetShaderImage(
                    PSO, EPipelineStage::PS_Compute,
                    "PrefilteredTexture", 
                    PrefilteredTextureMips[MipIndex].get(), 
                    EDataAccessFlag::DA_WriteOnly);
                RHICmdList->RHISetShaderUniformBuffer(
                    PSO, EPipelineStage::PS_Compute, 
                    "PrefilteredEnvTextureShaderBlock", PrefilterShaderUniformBuffer->GetRHI());
                RHICmdList->RHIDispatch(
                    (PrefilteredTexture->GetSizeX()/32) >> MipIndex, 
                    (PrefilteredTexture->GetSizeY()/32) >> MipIndex, 
                    1);
            }
            
        }

        SceneProxy->bHasData = true;
#endif
    }

    FReflectionProbeSceneProxy* UReflectionProbeComponent::CreateSceneProxy()
    {
        return new FReflectionProbeSceneProxy(this);
    }

    void UReflectionProbeComponent::SendRenderTransform()
    {
        if (SceneProxy)
        {
            dvec3 NewLocation = GetComponentLocation();
            auto Proxy = SceneProxy;
            ENQUEUE_RENDER_COMMAND(UReflectionProbeComponent_SendRenderTransform)(
                [NewLocation, Proxy](RenderGraph&) 
                {
                    Proxy->Location = NewLocation;
                });
        }
        USceneCaptureComponentCube::SendRenderTransform();
    }

    void UReflectionProbeComponent::SendRenderDynamicData()
    {
        if (SceneProxy)
        {
            auto NewExtent = Extent;
            auto NewOffset = OriginOffset;
            auto Proxy = SceneProxy;
            ENQUEUE_RENDER_COMMAND(UReflectionProbeComponent_SendRenderDynamicData)(
                [NewExtent, NewOffset, Proxy](RenderGraph&) 
                {
                    Proxy->Extent = NewExtent;
                    Proxy->OriginOffset = NewOffset;
                });
        }
        USceneCaptureComponentCube::SendRenderDynamicData();
    }

    FReflectionProbeSceneProxy::FReflectionProbeSceneProxy(UReflectionProbeComponent* Component)
        : Extent(Component->GetExtent())
        , OriginOffset(Component->GetOriginOffset())
        , Location(Component->GetComponentLocation())
        , ReflectionProbeSceneInfo(nullptr)
    {
        if (Component->IrradianceTexture && 
            Component->IrradianceTexture->GetResource())
        {
            IrradianceTexture = Component->IrradianceTexture->GetResource()->GetSamplerRHI();
        }

        if (Component->PrefilteredTexture && 
            Component->PrefilteredTexture->GetResource())
        {
            PrefilteredTexture = Component->PrefilteredTexture->GetResource()->GetSamplerRHI();
        }
    }

}