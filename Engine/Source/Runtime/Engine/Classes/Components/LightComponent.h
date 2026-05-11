#pragma once
#include "Light.generated.h"
#include "UniformBuffer.h"
#include "Frustum.h"
#include "SceneComponent.h"
#include "SceneView.h"

namespace nilou {

    class FLightSceneInfo;

    enum class ELightType {
        LT_None = 0,
        LT_Spot = 1,
        LT_Directional = 2,
        LT_Point = 3
    };
    
    enum EAttenCurveType {
        ACT_None = 0, 

        ACT_Linear = 1,

        ACT_Smooth = 2,
        
        ACT_Inverse = 3,

        ACT_InverseSquare = 4
    };
    
    struct FAttenCurve {
        EAttenCurveType type{ EAttenCurveType::ACT_None };
        float scale;
        union AttenCurveParams {

            // float atten = scale * (end_atten - t) / (end_atten - begin_atten)
            struct LinearParam {
                float begin_atten;
                float end_atten;
            } linear_params;

            
            // float linear = LinearAtten(t, begin_atten, end_atten);
            // float atten = scale * 3.0f * pow(linear,2.0f) - 2.0f * pow(linear,3.0f);
            struct SmoothParam {
                float begin_atten;
                float end_atten;
            } smooth_params;

            
            // float atten = scale / ( (kl*t) + (kc*scale) ) + offset;
            struct InverseParam {
                float offset;
                float kl;
                float kc;
            } inverse_params;


            // float atten = pow(scale,2.0f) / ( kq*pow(t,2.0f) + kl*t*scale + kc*pow(scale,2.0f) ) + offset;
            struct InverseSquareParam {
                float offset;
                float kq;       // MUST >0
                float kl;
                float kc;
            } inverse_squre_params;
        } u;

        FAttenCurve()
            : type(EAttenCurveType::ACT_InverseSquare)
            , scale(1.f)
        {
            u.inverse_squre_params.offset = 0.0f;
            u.inverse_squre_params.kq = 1.f;
            u.inverse_squre_params.kl = 0.f;
            u.inverse_squre_params.kc = 0.f;
        }
    };

    // struct FLightParameters
    // {
    //     vec4   LightIntensity;
    //     ivec2  ShadowMapResolution;
    //     FAttenCurve LightDistAttenuation;
    //     FAttenCurve LightAngleAttenuation;
    //     ELightType  LightType;
    //     float       Intensity;
    //     float       NearClipDistance;
    //     float       FarClipDistance;
    //     bool        bCastShadow;
    // };

    class NCLASS ULightComponent : public USceneComponent
    {
        GENERATED_BODY()
        friend class FLightSceneProxy;

        // For directional light, the unit is lux
        // For other types, the unit is cd
        DEFINE_DYNAMIC_DATA(FVector3f,   LightIntensity)
        DEFINE_DYNAMIC_DATA(FIntVector2, ShadowMapResolution)
        DEFINE_DYNAMIC_DATA(FAttenCurve, LightDistAttenuation)
        DEFINE_DYNAMIC_DATA(FAttenCurve, LightAngleAttenuation)
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

        void SetLightDistAttenParams(const FAttenCurve &AttenCurveParam);

        void SetLightAngleAttenParams(const FAttenCurve &AttenCurveParam);

        void UpdateUniformBuffer(RenderGraph& Graph);

        FVector Position;

        FVector3f Direction;

        FVector3f Up;

        ELightType LightType;

        FVector3f LightIntensity;

        FIntVector2 ShadowMapResolution;

        FAttenCurve DistAttenCurve;

        FAttenCurve AngleAttenCurve;

        bool bCastShadow;

        float ScreenAspect;

        float VerticalFieldOfView;
    
        FLightSceneInfo *LightSceneInfo;

        TParameterBlockRef<shader::Light> LightParams = nullptr;
    };

}
