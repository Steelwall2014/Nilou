#include "ReflectionProbeComponent.h"
#include "Common/ContentManager.h"
#include "TextureCube.h"

namespace nilou {

    DECLARE_GLOBAL_SHADER(FIrradianceEnvTextureShader);
    IMPLEMENT_SHADER_TYPE(FIrradianceEnvTextureShader, "/Shaders/GlobalShaders/IrradianceEnvTextureShader.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FPrefilteredEnvTextureShader);
    IMPLEMENT_SHADER_TYPE(FPrefilteredEnvTextureShader, "/Shaders/GlobalShaders/PrefilteredEnvTextureShader.comp", EShaderFrequency::SF_Compute, Global)



    void UReflectionProbeComponent::UpdateSceneCaptureContents(FScene* Scene)
    {
        USceneCaptureComponentCube::UpdateSceneCaptureContents(Scene);

        ENQUEUE_RENDER_COMMAND(UReflectionProbeComponent_UpdateSceneCaptureContents)(
            [this, Scene](FDynamicRHI* RHICmdList) 
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

    }

    void UReflectionProbeComponent::OnUnregister()
    {
        BeginReleaseResource(IrradianceShaderUniformBuffer.get());
        BeginReleaseResource(PrefilterShaderUniformBuffer.get());
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

            for (int i = 0; i < 5; i++)
            {
                PrefilterShaderUniformBuffer->Data.TextureSize = TextureTarget->GetSizeX() >> i;
                PrefilterShaderUniformBuffer->Data.roughness = i * 0.25;
                PrefilterShaderUniformBuffer->UpdateUniformBuffer();
                RHITexture* CubeMap = PrefilteredTexture->GetResource()->TextureRHI.get();
                RHITextureCubeRef TextureView = RHICmdList->RHICreateTextureViewCube(
                    CubeMap, CubeMap->GetFormat(), 
                    i, 1);
                RHICmdList->RHISetShaderImage(
                    PSO, EPipelineStage::PS_Compute,
                    "PrefilteredTexture", 
                    TextureView.get(), 
                    EDataAccessFlag::DA_WriteOnly);
                RHICmdList->RHISetShaderUniformBuffer(
                    PSO, EPipelineStage::PS_Compute, 
                    "PrefilteredEnvTextureShaderBlock", PrefilterShaderUniformBuffer->GetRHI());
                RHICmdList->RHIDispatch(32 >> i, 32 >> i, 1);
            }
            
        }

    }

}