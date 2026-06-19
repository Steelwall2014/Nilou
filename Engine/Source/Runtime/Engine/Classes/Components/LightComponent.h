#pragma once
#include "Light.generated.h"
#include "UniformBuffer.h"
#include "Frustum.h"
#include "SceneComponent.h"
#include "SceneView.h"

namespace nilou {

    class FLightSceneInfo;

    using ELightType = shader::ELightType;
    using ELightAttenuationCurveType = shader::ELightAttenuationCurveType;
    using FLightAttenuationCurve = shader::LightAttenuationCurve<Std140Layout>;

    class NCLASS ULightComponent : public USceneComponent
    {
        GENERATED_BODY()
        friend class FLightSceneProxy;

        // For directional light, the unit is lux
        // For other types, the unit is cd
        DEFINE_DYNAMIC_DATA(FVector3f,   LightIntensity)
        DEFINE_DYNAMIC_DATA(FIntVector2, ShadowMapResolution)
        DEFINE_DYNAMIC_DATA(FLightAttenuationCurve, LightDistAttenuation)
        DEFINE_DYNAMIC_DATA(FLightAttenuationCurve, LightAngleAttenuation)
        // DEFINE_DYNAMIC_DATA(ELightType,  LightType)
        DEFINE_DYNAMIC_DATA(bool,        bCastShadow)
        
    public:
        ULightComponent();

		inline void SetLightType(const ELightType &InLightType) { LightType = InLightType; MarkRenderStateDirty(); } 
		inline ELightType GetLightType() const { return LightType; } 

        virtual class FLightSceneProxy *CreateSceneProxy();

        virtual void CreateRenderState() override;

        virtual void DestroyRenderState() override;

        virtual void SendRenderTransform() override;

        virtual void SendRenderDynamicData() override;

        class FLightSceneProxy *SceneProxy;

    private:

        ELightType LightType;
    };

    class FLightSceneProxy
    {
        friend class FScene;
        friend class FDeferredShadingSceneRenderer;
    public:
        FLightSceneProxy(ULightComponent *InComponent);
        virtual ~FLightSceneProxy();

        FLightSceneInfo *GetLightSceneInfo() { return LightSceneInfo; }

        void SetPositionAndDirection(const FVector &InPosition, const FVector3f &InDirection, const FVector3f &InUp);

        void SetCastShadow(bool bCastShadow);

        void SetLightIntensity(const FVector3f &LightIntensity);

        void SetLightType(ELightType LightType);

        void SetShadowMapResolution(FIntVector2 ShadowMapResolution);

        void SetLightDistAttenParams(const FLightAttenuationCurve &AttenCurveParam);

        void SetLightAngleAttenParams(const FLightAttenuationCurve &AttenCurveParam);

        void UpdateUniformBuffer(RenderGraph& Graph);

        FVector Position;

        FVector3f Direction;

        FVector3f Up;

        ELightType LightType;

        FVector3f LightIntensity;

        FIntVector2 ShadowMapResolution;

        FLightAttenuationCurve DistAttenCurve;

        FLightAttenuationCurve AngleAttenCurve;

        bool bCastShadow;

        float ScreenAspect;

        float VerticalFieldOfView;
    
        FLightSceneInfo *LightSceneInfo;

        TParameterBlockRef<shader::Light> LightParams = nullptr;
    };

}
