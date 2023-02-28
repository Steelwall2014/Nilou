#pragma once
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
    //     glm::vec4   LightColor;
    //     glm::ivec2  ShadowMapResolution;
    //     FAttenCurve LightDistAttenuation;
    //     FAttenCurve LightAngleAttenuation;
    //     ELightType  LightType;
    //     float       Intensity;
    //     float       NearClipDistance;
    //     float       FarClipDistance;
    //     bool        bCastShadow;
    // };

    UCLASS()
    class ULightComponent : public USceneComponent
    {
        GENERATE_CLASS_INFO()
        friend class FLightSceneProxy;

        DEFINE_DYNAMIC_DATA(glm::vec4,   LightColor)
        DEFINE_DYNAMIC_DATA(glm::ivec2,  ShadowMapResolution)
        DEFINE_DYNAMIC_DATA(FAttenCurve, LightDistAttenuation)
        DEFINE_DYNAMIC_DATA(FAttenCurve, LightAngleAttenuation)
        // DEFINE_DYNAMIC_DATA(ELightType,  LightType)
        DEFINE_DYNAMIC_DATA(float,       Intensity)
        DEFINE_DYNAMIC_DATA(bool,        bCastShadow)
        
    public:
        ULightComponent(AActor *InOwner = nullptr);

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

    // UCLASS()
    // class USpotLightComponnet : public ULightComponent
    // {
    //     GENERATE_CLASS_INFO()
    // public:
    //     USpotLightComponnet(AActor *InOwner) : ULightComponent(InOwner) { LightParameters.LightType = ELightType::LT_Spot; }
    //     virtual class FLightSceneProxy *CreateSceneProxy();

    // };

    // UCLASS()
    // class UDirectionalLightComponent : public ULightComponent
    // {
    //     GENERATE_CLASS_INFO()
    // public:
    //     UDirectionalLightComponent(AActor *InOwner) : ULightComponent(InOwner) { LightParameters.LightType = ELightType::LT_Directional; }
    //     virtual class FLightSceneProxy *CreateSceneProxy();

    //     // virtual glm::mat4 GetLightProjectionMatrix();

    // };
    // class FScene;
    // class ULightComponent;

    constexpr int CASCADED_SHADOWMAP_SPLIT_COUNT = 8;

    BEGIN_UNIFORM_BUFFER_STRUCT(FLightAttenParameters)
        SHADER_PARAMETER(vec4, AttenCurveParams)
        SHADER_PARAMETER(float, AttenCurveScale)
        SHADER_PARAMETER(int, AttenCurveType)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FShadowMappingParameters)
        SHADER_PARAMETER(dmat4, WorldToClip)
        SHADER_PARAMETER(ivec2, Resolution)
        SHADER_PARAMETER(float, FrustumFar)
    END_UNIFORM_BUFFER_STRUCT()

    template<int N>
    BEGIN_UNIFORM_BUFFER_STRUCT(FShadowMappingBlock)
        SHADER_PARAMETER_STRUCT_ARRAY(FShadowMappingParameters, N, Frustums)
    END_UNIFORM_BUFFER_STRUCT()

    // Manually assign the alignment.
    // There is only a single int member in the struct, so the alignment should be 16 instead of 4
    BEGIN_UNIFORM_BUFFER_STRUCT(FShadowMapFrustumIndex)
        alignas(16) int FrustumIndex;
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FLightShaderParameters)
        SHADER_PARAMETER_STRUCT(FLightAttenParameters, lightDistAttenParams)
        SHADER_PARAMETER_STRUCT(FLightAttenParameters, lightAngleAttenParams)
        SHADER_PARAMETER(dvec3, lightPosition)
        SHADER_PARAMETER(vec4, lightColor)
        SHADER_PARAMETER(vec3, lightDirection)
        SHADER_PARAMETER(int, lightType) 
        SHADER_PARAMETER(float, lightIntensity)
        SHADER_PARAMETER(int, lightCastShadow)
    END_UNIFORM_BUFFER_STRUCT()

    class FLightSceneProxy
    {
        friend class FScene;
        friend class FDefferedShadingSceneRenderer;
    public:
        FLightSceneProxy(ULightComponent *InComponent);

        FLightSceneInfo *GetLightSceneInfo() { return LightSceneInfo; }

        void SetPositionAndDirection(const glm::dvec3 &InPosition, const glm::vec3 &InDirection, const glm::vec3 &InUp);

        void SetLightIntensity(float Intensity);

        void SetCastShadow(bool CastShadow);

        void SetLightColor(const glm::vec4 &LightColor);

        void SetLightType(ELightType LightType);

        void SetShadowMapResolution(glm::ivec2 ShadowMapResolution);

        void SetLightDistAttenParams(const FAttenCurve &AttenCurveParam);

        void SetLightAngleAttenParams(const FAttenCurve &AttenCurveParam);

        void UpdateUniformBuffer();

        FSceneLightView GetSceneLightView() const;

        glm::dvec3 Position;

        glm::vec3 Direction;

        glm::vec3 Up;

        ELightType LightType;

        glm::ivec2 ShadowMapResolution;

        float ScreenAspect;

        float VerticalFieldOfView;

    protected:
    
        FLightSceneInfo *LightSceneInfo;

        void SetLightAttenParams(FLightAttenParameters &OutParameter, const FAttenCurve &AttenCurveParam);

        TUniformBufferRef<FLightShaderParameters> LightUniformBufferRHI;
    };

    // class FSpotLightSceneProxy : public FLightSceneProxy
    // {
    // public:
    //     // virtual glm::mat4 GetLightProjectionMatrix();

    // };

    // class FDirectionalLightSceneProxy : public FLightSceneProxy
    // {
    // public:
    //     // virtual glm::mat4 GetLightProjectionMatrix();

    // };

}
