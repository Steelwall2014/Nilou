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

    ULightComponent::ULightComponent()
        : LightType(ELightType::LT_Directional)
        , LightIntensity(vec3(1.474000, 1.850400, 1.911980)*10.f)
        , bCastShadow(true)
        , ShadowMapResolution(ivec2(2048))
    {

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
        ENQUEUE_RENDER_COMMAND(ULightComponent_SendRenderTransform)(
            [this](FDynamicRHI*) 
            {
                SceneProxy->SetPositionAndDirection(GetComponentLocation(), GetForwardVector(), GetUpVector());
            });

        USceneComponent::SendRenderTransform();
    }

    void ULightComponent::SendRenderDynamicData()
    {
        ENQUEUE_RENDER_COMMAND(ULightComponent_SendRenderDynamicData)(
            [this](FDynamicRHI*) 
            {
                SceneProxy->SetLightDistAttenParams(LightDistAttenuation);
                SceneProxy->SetLightAngleAttenParams(LightAngleAttenuation);
                SceneProxy->SetLightIntensity(LightIntensity);
                SceneProxy->SetCastShadow(bCastShadow);
                SceneProxy->SetShadowMapResolution(ShadowMapResolution);
            });

        USceneComponent::SendRenderDynamicData();
    }

    FLightSceneProxy::FLightSceneProxy(ULightComponent *InComponent)
        : LightSceneInfo(nullptr)
    { 
        InComponent->SceneProxy = this;
        SetLightDistAttenParams(InComponent->LightDistAttenuation);
        SetLightAngleAttenParams(InComponent->LightAngleAttenuation);
        SetPositionAndDirection(InComponent->GetComponentLocation(), InComponent->GetForwardVector(), InComponent->GetUpVector());
        SetLightIntensity(InComponent->LightIntensity);
        SetCastShadow(InComponent->bCastShadow);
        SetLightType(InComponent->LightType);
        SetShadowMapResolution(InComponent->GetShadowMapResolution());
    }

    void FLightSceneProxy::SetPositionAndDirection(const glm::dvec3 &InPosition, const vec3 &InDirection, const vec3 &InUp)
    {
        Position = InPosition;
        Direction = InDirection;
        Up = InUp;
    }

    void FLightSceneProxy::SetLightIntensity(const vec3& InLightIntensity)
    {
        LightIntensity = InLightIntensity;
    }

    void FLightSceneProxy::SetCastShadow(bool InCastShadow)
    {
        bCastShadow = InCastShadow;
    }

    void FLightSceneProxy::SetLightType(ELightType InLightType)
    {
        LightType = InLightType;
    }

    void FLightSceneProxy::SetShadowMapResolution(ivec2 InShadowMapResolution)
    {
        ShadowMapResolution = InShadowMapResolution;
        ScreenAspect = (float)ShadowMapResolution.x / (float)ShadowMapResolution.y;
    }

    void FLightSceneProxy::SetLightDistAttenParams(const FAttenCurve &AttenCurveParam) 
    { 
    }

    void FLightSceneProxy::SetLightAngleAttenParams(const FAttenCurve &AttenCurveParam)
    {
        VerticalFieldOfView = glm::radians(90.0);
    }
}
