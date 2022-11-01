#include <cmath>
#include <iostream>

#include "Common/Maths.h"
#include "UniformBuffer.h"
#include "LightComponent.h"
// #include "LightSceneProxy.h"
#include "Common/World.h"
#include "Common/Log.h"

namespace nilou {

    // IMPLEMENT_UNIFORM_BUFFER_STRUCT(FLightAttenParameters)
    // IMPLEMENT_UNIFORM_BUFFER_STRUCT(FShadowMappingParameters)
    // IMPLEMENT_UNIFORM_BUFFER_STRUCT(FLightShaderParameters)

    ULightComponent::ULightComponent(AActor *InOwner)
        : USceneComponent(InOwner)
        , LightType(ELightType::LT_Spot)
        , LightColor(vec4(1.0f, 1.0f, 1.0f, 1.0f))
        , Intensity(10.f)
        , bCastShadow(true)
        , NearClipDistance(1.f)
        , FarClipDistance(1000000.f)
        , ShadowMapResolution(ivec2(1024))
    {

    }

    glm::vec4 ULightComponent::GetLightColor()
    {
        return LightColor;
    }

    float ULightComponent::GetIntensity()
    {
        return Intensity;
    }

    bool ULightComponent::GetCastShadows()
    {
        return bCastShadow;
    }

    float ULightComponent::GetNearClip()
    {
        return NearClipDistance;
    }

    float ULightComponent::GetFarClip()
    {
        return FarClipDistance;
    }

    void ULightComponent::SetDistAttenCurve(FAttenCurve dist_atten_angle)
    {
        LightDistAttenuation = dist_atten_angle;
        MarkRenderDynamicDataDirty();
    }

    void ULightComponent::SetLightColor(glm::vec4 light_color)
    {
        LightColor = light_color;
        MarkRenderDynamicDataDirty();
    }

    FAttenCurve ULightComponent::GetDistAttenCurve()
    {
        return LightDistAttenuation;
    }

    glm::ivec2 ULightComponent::GetShadowMapResolution()
    {
        return ShadowMapResolution;
    }

    FLightSceneProxy *ULightComponent::CreateSceneProxy()
    {
        return new FLightSceneProxy(this);
    }

    void ULightComponent::CreateRenderState()
    {
        USceneComponent::CreateRenderState();
        UWorld* World = GetWorld();
        if(World && World->Scene)
        {
            World->Scene->AddLight(this);
        }
    }

    void ULightComponent::DestroyRenderState()
    {
        UWorld* World = GetWorld();
        if(World && World->Scene)
        {
            World->Scene->RemoveLight(this);
        }

        USceneComponent::DestroyRenderState();
    }

    void ULightComponent::SendRenderTransform()
    {
        SceneProxy->SetPositionAndDirection(GetComponentLocation(), GetForwardVector());

        USceneComponent::SendRenderTransform();
    }

    void ULightComponent::SendRenderDynamicData()
    {
        SceneProxy->SetLightDistAttenParams(LightDistAttenuation);
        SceneProxy->SetLightAngleAttenParams(LightAngleAttenuation);
        SceneProxy->SetPositionAndDirection(GetComponentLocation(), GetForwardVector());
        SceneProxy->SetLightColor(LightColor);
        SceneProxy->SetLightIntensity(Intensity);
        SceneProxy->SetCastShadow(bCastShadow);

        USceneComponent::SendRenderDynamicData();
    }

    FLightSceneProxy::FLightSceneProxy(ULightComponent *InComponent)
        : LightSceneInfo(nullptr)
    { 
        InComponent->SceneProxy = this;
        LightUniformBufferRHI = CreateUniformBuffer<FLightShaderParameters>();
        LightUniformBufferRHI->Data.ShadowMappingStartIndex = 0;
        LightUniformBufferRHI->Data.ShadowMappingEndIndex = 0;
        SetLightDistAttenParams(InComponent->LightDistAttenuation);
        SetLightAngleAttenParams(InComponent->LightAngleAttenuation);
        SetPositionAndDirection(InComponent->GetComponentLocation(), InComponent->GetForwardVector());
        SetLightColor(InComponent->LightColor);
        SetLightIntensity(InComponent->Intensity);
        SetCastShadow(InComponent->bCastShadow);
        SetLightType(InComponent->LightType);
        SetShadowMapResolution(InComponent->GetShadowMapResolution());
        // LightUniformBufferRHI->InitRHI();
        BeginInitResource(LightUniformBufferRHI.get());
    }

    void FLightSceneProxy::SetPositionAndDirection(const glm::vec3 &InPosition, const glm::vec3 &InDirection)
    {
        Position = InPosition;
        Direction = InDirection;
        // LightUniformBufferRHI->Set("lightPosition", InPosition);
        // LightUniformBufferRHI->Set("lightDirection", InDirection);
        LightUniformBufferRHI->Data.lightPosition = InPosition;
        LightUniformBufferRHI->Data.lightDirection = InDirection;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetLightIntensity(float Intensity)
    {
        // LightUniformBufferRHI->Set("lightIntensity", Intensity);
        LightUniformBufferRHI->Data.lightIntensity = Intensity;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetCastShadow(bool CastShadow)
    {
        // LightUniformBufferRHI->Set("lightCastShadow", CastShadow);
        LightUniformBufferRHI->Data.lightCastShadow = CastShadow;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetLightColor(const glm::vec4 &LightColor)
    {
        LightUniformBufferRHI->Data.lightColor = LightColor;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetLightType(ELightType InLightType)
    {
        LightType = InLightType;
        LightUniformBufferRHI->Data.lightType = (int)InLightType;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetShadowMapResolution(glm::ivec2 InShadowMapResolution)
    {
        ShadowMapResolution = InShadowMapResolution;
        ScreenAspect = (float)ShadowMapResolution.x / (float)ShadowMapResolution.y;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetFrustumClipDistances(float Near, float Far)
    {
        NearClipDistance = Near;
        FarClipDistance = Far;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetLightDistAttenParams(const FAttenCurve &AttenCurveParam) 
    { 
        SetLightAttenParams(LightUniformBufferRHI->Data.lightDistAttenParams, AttenCurveParam); 
    }

    void FLightSceneProxy::SetLightAngleAttenParams(const FAttenCurve &AttenCurveParam)
    {
        SetLightAttenParams(LightUniformBufferRHI->Data.lightAngleAttenParams, AttenCurveParam);
        float ScreenAspect = (float)ShadowMapResolution.x / (float)ShadowMapResolution.y;
        float fovy = glm::radians(90.f);
        switch (AttenCurveParam.type) {
            case EAttenCurveType::ACT_Linear:
            case EAttenCurveType::ACT_Smooth:
                fovy = AttenCurveParam.u.linear_params.end_atten;
                break;
            case EAttenCurveType::ACT_Inverse:
            {
                float kl = AttenCurveParam.u.inverse_params.kl;
                float kc = AttenCurveParam.u.inverse_params.kc;
                float offset = AttenCurveParam.u.inverse_params.offset;
                float scale = AttenCurveParam.scale;
                if (offset == 0)
                    fovy = ( scale / ( KINDA_SMALL_NUMBER-offset ) - kc*scale) / kl;
            }
            case EAttenCurveType::ACT_InverseSquare:
            {
                float kl = AttenCurveParam.u.inverse_squre_params.kl;
                float kc = AttenCurveParam.u.inverse_squre_params.kc;
                float kq = AttenCurveParam.u.inverse_squre_params.kq;
                float offset = AttenCurveParam.u.inverse_squre_params.offset;
                float scale = AttenCurveParam.scale;
                // if (offset > 0)
                // {
                    float under_sqrt = kl*kl - 4.f*kq*(1.f/offset+kc);
                    if (under_sqrt >= 0)
                        fovy = ( sqrt(under_sqrt)*scale - kl*scale ) / (2*kq);
                    else 
                        NILOU_LOG(Error, "Invalid Light Angle Attenuation Parameter: Can't attenuate to zero");
                // }
                // else {
                //     float under_sqrt = kl*kl - 4.f*kq*(1.f/offset+kc);
                //     if (under_sqrt >= 0)
                //         fovy = ( sqrt(under_sqrt)*scale - kl*scale ) / (2*kq);
                //     else 
                //         std::cout << "Invalid Light Angle Attenuation Parameter: Can't attenuate to zero";
                // }
            }
        }

        ComputedFOVY = fovy;
    }

    void FLightSceneProxy::SetLightAttenParams(FLightAttenParameters &OutParameter, const FAttenCurve &AttenCurveParam)
    {
        EAttenCurveType CurveType = AttenCurveParam.type;
        OutParameter.AttenCurveType = (int)AttenCurveParam.type;
        OutParameter.AttenCurveScale = AttenCurveParam.scale;
        switch (CurveType) 
        {
            case EAttenCurveType::ACT_Linear:
                OutParameter.AttenCurveParams =
                    glm::vec4(
                        AttenCurveParam.u.linear_params.begin_atten, 
                        AttenCurveParam.u.linear_params.end_atten, 
                        0, 0);
                break;
            case EAttenCurveType::ACT_Smooth:
                OutParameter.AttenCurveParams =
                    glm::vec4(
                        AttenCurveParam.u.smooth_params.begin_atten, 
                        AttenCurveParam.u.smooth_params.end_atten, 
                        0, 0);
                break;
            case EAttenCurveType::ACT_Inverse:
                OutParameter.AttenCurveParams =
                    glm::vec4(
                        AttenCurveParam.u.inverse_params.offset, 
                        AttenCurveParam.u.inverse_params.kl, 
                        AttenCurveParam.u.inverse_params.kc, 0);
                break;
            case EAttenCurveType::ACT_InverseSquare:
                OutParameter.AttenCurveParams =
                    glm::vec4(
                        AttenCurveParam.u.inverse_squre_params.offset,
                        AttenCurveParam.u.inverse_squre_params.kq,
                        AttenCurveParam.u.inverse_squre_params.kl,
                        AttenCurveParam.u.inverse_squre_params.kc);
                break;
        }
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }
    // FLightSceneProxy *USpotLightComponnet::CreateSceneProxy()
    // {
        
    // }

    // FLightSceneProxy *UDirectionalLightComponent::CreateSceneProxy()
    // {
        
    // }
}
