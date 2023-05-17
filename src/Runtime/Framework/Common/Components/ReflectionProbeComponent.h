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
            , Extent(dvec3(1))
            , OriginOffset(dvec3(0))
        { }

        class UTextureCube* IrradianceTexture;

        class UTextureCube* PrefilteredTexture;

        void SetExtent(dvec3 NewExtent)
        {
            if (NewExtent != Extent)
            {
                Extent = NewExtent;
                MarkRenderDynamicDataDirty();
            }
        }

        void SetOriginOffset(dvec3 NewOriginOffset)
        {
            if (NewOriginOffset != OriginOffset)
            {
                OriginOffset = NewOriginOffset;
                MarkRenderDynamicDataDirty();
            }
        }

        dvec3 GetExtent() const
        {
            return Extent;
        }

        dvec3 GetOriginOffset() const
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

    protected:

        dvec3 Extent;

        dvec3 OriginOffset;

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

    class FReflectionProbeSceneProxy
    {
    public:
        FReflectionProbeSceneProxy(UReflectionProbeComponent* Component);

        dvec3 Extent;

        dvec3 OriginOffset;

        dvec3 Location;

        FRHISampler* IrradianceTexture;

        FRHISampler* PrefilteredTexture;

        class FReflectionProbeSceneInfo* ReflectionProbeSceneInfo;

    };

}