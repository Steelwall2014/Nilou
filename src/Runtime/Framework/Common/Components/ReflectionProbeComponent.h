#pragma once

#include "SceneCaptureComponent.h"

namespace nilou {

    class NCLASS UReflectionProbeComponent : public USceneCaptureComponentCube
    {
        GENERATED_BODY()
    public:

        UReflectionProbeComponent() 
            : IrradianceTexture(nullptr)
            , PrefilteredTexture(nullptr)
            , SceneProxy(nullptr)
            , Extent(FVector(1))
            , OriginOffset(FVector(0))
        { }

        class UTextureCube* IrradianceTexture;

        class UTextureCube* PrefilteredTexture;

        void SetExtent(FVector NewExtent)
        {
            if (NewExtent != Extent)
            {
                Extent = NewExtent;
                MarkRenderDynamicDataDirty();
            }
        }

        void SetOriginOffset(FVector NewOriginOffset)
        {
            if (NewOriginOffset != OriginOffset)
            {
                OriginOffset = NewOriginOffset;
                MarkRenderDynamicDataDirty();
            }
        }

        FVector GetExtent() const
        {
            return Extent;
        }

        FVector GetOriginOffset() const
        {
            return OriginOffset;
        }

        virtual void UpdateSceneCaptureContents(FScene* Scene) override;

        virtual void OnRegister() override;

        virtual void OnUnregister() override;

        virtual class FReflectionProbeSceneProxy* CreateSceneProxy();

        virtual void SendRenderTransform() override;

        virtual void SendRenderDynamicData() override;

        FReflectionProbeSceneProxy* SceneProxy;
  
        BEGIN_UNIFORM_BUFFER_STRUCT(IrradianceEnvTextureShaderBlock)
            alignas(16) NPROPERTY() int TextureSize;
        END_UNIFORM_BUFFER_STRUCT()

        BEGIN_UNIFORM_BUFFER_STRUCT(PrefilteredEnvTextureShaderBlock)
            SHADER_PARAMETER(int, TextureSize);
            SHADER_PARAMETER(float, roughness);
        END_UNIFORM_BUFFER_STRUCT()

    protected:

        FVector Extent;

        FVector OriginOffset;

        void UpdateSceneCaptureContents_RenderThread(FScene* Scene);

        std::vector<RHITextureCubeRef> PrefilteredTextureMips;

        TRDGUniformBufferRef<IrradianceEnvTextureShaderBlock> IrradianceShaderUniformBuffer;
        TRDGUniformBufferRef<PrefilteredEnvTextureShaderBlock> PrefilterShaderUniformBuffer;

    };

    class FReflectionProbeSceneProxy
    {
    public:
        FReflectionProbeSceneProxy(UReflectionProbeComponent* Component);

        FVector Extent;

        FVector OriginOffset;

        FVector Location;

        RHISampler IrradianceTexture;

        RHISampler PrefilteredTexture;

        bool bHasData = false;

        class FReflectionProbeSceneInfo* ReflectionProbeSceneInfo;

    };

}