#include "ReflectionProbeComponent.h"
#include "Common/ContentManager.h"
#include "TextureCube.h"
#include "TextureRenderTarget.h"
#include "DefferedShadingSceneRenderer.h"

namespace nilou {

    DECLARE_GLOBAL_SHADER(FIrradianceEnvTextureShader);
    IMPLEMENT_SHADER_TYPE(FIrradianceEnvTextureShader, "/Shaders/GlobalShaders/IrradianceEnvTextureShader.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FPrefilteredEnvTextureShader);
    IMPLEMENT_SHADER_TYPE(FPrefilteredEnvTextureShader, "/Shaders/GlobalShaders/PrefilteredEnvTextureShader.comp", EShaderFrequency::SF_Compute, Global)



    void UReflectionProbeComponent::UpdateSceneCaptureContents(FScene* Scene)
    {
        UpdateSceneCaptureContents_Internal(Scene, GetComponentLocation()+OriginOffset);
        ENQUEUE_RENDER_COMMAND(UReflectionProbeComponent_UpdateSceneCaptureContents)(
            [Scene, this](FDynamicRHI* RHICmdList) 
            {
                UpdateSceneCaptureContents_RenderThread(Scene, RHICmdList);
            });
    }

    void UReflectionProbeComponent::OnRegister()
    {
        USceneCaptureComponentCube::OnRegister();
        
        IrradianceShaderUniformBuffer = CreateUniformBuffer<IrradianceEnvTextureShaderBlock>();
        PrefilterShaderUniformBuffer = CreateUniformBuffer<PrefilteredEnvTextureShaderBlock>();
        BeginInitResource(IrradianceShaderUniformBuffer.get());
        BeginInitResource(PrefilterShaderUniformBuffer.get());

        if (WorldPrivate) 
        {
            if (WorldPrivate->Scene)
            {
                WorldPrivate->Scene->AddReflectionProbe(this);
            }
        }

    }

    void UReflectionProbeComponent::OnUnregister()
    {
        BeginReleaseResource(IrradianceShaderUniformBuffer.get());
        BeginReleaseResource(PrefilterShaderUniformBuffer.get());

        if (WorldPrivate) 
        {
            if (WorldPrivate->Scene)
            {
                WorldPrivate->Scene->RemoveReflectionProbe(this);
            }
        }

        USceneCaptureComponentCube::OnUnregister();
    }

    void UReflectionProbeComponent::UpdateSceneCaptureContents_RenderThread(FScene* Scene, FDynamicRHI* RHICmdList)
    {

        {
            
            IrradianceShaderUniformBuffer->Data.TextureSize = IrradianceTexture->GetSizeX();
            IrradianceShaderUniformBuffer->UpdateUniformBuffer();

            FShaderPermutationParameters PermutationParameters(&FIrradianceEnvTextureShader::StaticType, 0);
            FShaderInstance *IrradianceShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(IrradianceShader);

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
            FShaderInstance *PrefilterShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(PrefilterShader);

            RHICmdList->RHISetShaderSampler(
                PSO, EPipelineStage::PS_Compute,
                "EnvironmentTexture", *TextureTarget->GetResource()->GetSamplerRHI());

            int NumMips = PrefilteredTexture->NumMips;
            float delta_roughness = 1.0 / glm::max(NumMips-1, 1);
            for (int MipIndex = 0; MipIndex < NumMips; MipIndex++)
            {
                PrefilterShaderUniformBuffer->Data.TextureSize = TextureTarget->GetSizeX() >> MipIndex;
                PrefilterShaderUniformBuffer->Data.roughness = MipIndex * delta_roughness;
                PrefilterShaderUniformBuffer->UpdateUniformBuffer();
                RHITexture* CubeMap = PrefilteredTexture->GetResource()->TextureRHI.get();
                RHITextureCubeRef TextureView = RHICmdList->RHICreateTextureViewCube(
                    CubeMap, CubeMap->GetFormat(), 
                    MipIndex, 1);
                RHICmdList->RHISetShaderImage(
                    PSO, EPipelineStage::PS_Compute,
                    "PrefilteredTexture", 
                    TextureView.get(), 
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
                [NewLocation, Proxy](FDynamicRHI*) 
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
                [NewExtent, NewOffset, Proxy](FDynamicRHI*) 
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