#pragma once

#include "SceneCaptureComponent.h"

namespace nilou {

    UCLASS()
    class UReflectionProbeComponent : public USceneCaptureComponentCube
    {
        GENERATE_CLASS_INFO()
    public:

        UReflectionProbeComponent(AActor *InOwner = nullptr) 
            : USceneCaptureComponentCube(InOwner)
            , IrradianceTexture(nullptr)
            , PrefilteredTexture(nullptr)
        { }

        class UTextureCube* IrradianceTexture;

        class UTextureCube* PrefilteredTexture;

        virtual void UpdateSceneCaptureContents(FScene* Scene) override;

        virtual void OnRegister() override;

        virtual void OnUnregister() override;

    protected:

        void UpdateSceneCaptureContents_RenderThread(FScene* Scene, FDynamicRHI* RHICmdList);
  
        BEGIN_UNIFORM_BUFFER_STRUCT(IrradianceEnvTextureShaderBlock)
            alignas(16) int TextureSize;
        END_UNIFORM_BUFFER_STRUCT()

        BEGIN_UNIFORM_BUFFER_STRUCT(PrefilteredEnvTextureShaderBlock)
            SHADER_PARAMETER(int, TextureSize);
            SHADER_PARAMETER(float, roughness);
        END_UNIFORM_BUFFER_STRUCT()

        TUniformBufferRef<IrradianceEnvTextureShaderBlock> IrradianceShaderUniformBuffer;
        TUniformBufferRef<PrefilteredEnvTextureShaderBlock> PrefilterShaderUniformBuffer;

    };

}