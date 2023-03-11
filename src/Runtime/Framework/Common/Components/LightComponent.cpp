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
        , LightIntensity(vec3(1.474000, 1.850400, 1.911980))
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
        SceneProxy->SetPositionAndDirection(GetComponentLocation(), GetForwardVector(), GetUpVector());

        USceneComponent::SendRenderTransform();
    }

    void ULightComponent::SendRenderDynamicData()
    {
        SceneProxy->SetLightDistAttenParams(LightDistAttenuation);
        SceneProxy->SetLightAngleAttenParams(LightAngleAttenuation);
        SceneProxy->SetLightIntensity(LightIntensity);
        SceneProxy->SetCastShadow(bCastShadow);
        SceneProxy->SetShadowMapResolution(ShadowMapResolution);

        USceneComponent::SendRenderDynamicData();
    }

    FLightSceneProxy::FLightSceneProxy(ULightComponent *InComponent)
        : LightSceneInfo(nullptr)
    { 
        InComponent->SceneProxy = this;
        LightUniformBufferRHI = CreateUniformBuffer<FLightShaderParameters>();
        SetLightDistAttenParams(InComponent->LightDistAttenuation);
        SetLightAngleAttenParams(InComponent->LightAngleAttenuation);
        SetPositionAndDirection(InComponent->GetComponentLocation(), InComponent->GetForwardVector(), InComponent->GetUpVector());
        SetLightIntensity(InComponent->LightIntensity);
        SetCastShadow(InComponent->bCastShadow);
        SetLightType(InComponent->LightType);
        SetShadowMapResolution(InComponent->GetShadowMapResolution());
        BeginInitResource(LightUniformBufferRHI.get());
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(false);
    }

    void FLightSceneProxy::SetPositionAndDirection(const glm::dvec3 &InPosition, const glm::vec3 &InDirection, const glm::vec3 &InUp)
    {
        Position = InPosition;
        Direction = InDirection;
        Up = InUp;
        LightUniformBufferRHI->Data.lightPosition = InPosition;
        LightUniformBufferRHI->Data.lightDirection = InDirection;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetLightIntensity(const vec3& LightIntensity)
    {
        LightUniformBufferRHI->Data.lightIntensity = LightIntensity;
        if (LightSceneInfo)
            LightSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FLightSceneProxy::SetCastShadow(bool CastShadow)
    {
        LightUniformBufferRHI->Data.lightCastShadow = CastShadow;
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

    void FLightSceneProxy::SetLightDistAttenParams(const FAttenCurve &AttenCurveParam) 
    { 
        SetLightAttenParams(LightUniformBufferRHI->Data.lightDistAttenParams, AttenCurveParam); 
    }

    void FLightSceneProxy::SetLightAngleAttenParams(const FAttenCurve &AttenCurveParam)
    {
        VerticalFieldOfView = glm::radians(90.0);
        SetLightAttenParams(LightUniformBufferRHI->Data.lightAngleAttenParams, AttenCurveParam);
    }

    void FLightSceneProxy::UpdateUniformBuffer()
    {
        ENQUEUE_RENDER_COMMAND(FLightSceneProxy_UpdateUniformBuffer)(
            [this](FDynamicRHI *DynamicRHI) 
            {
                LightUniformBufferRHI->UpdateUniformBuffer();
            });
        
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
