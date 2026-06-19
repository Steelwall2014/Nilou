#include <cmath>
#include <iostream>

#include "UniformBuffer.h"
#include "Components/LightComponent.h"
// #include "LightSceneProxy.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"

namespace nilou {

    static FLightAttenuationCurve MakeDefaultInverseSquareAttenCurve()
    {
        FLightAttenuationCurve curve{};
        curve.type = ELightAttenuationCurveType::InverseSquare;
        curve.scale = 1.f;
        curve.params = float4(0.f, 1.f, 0.f, 0.f);
        return curve;
    }

    // IMPLEMENT_UNIFORM_BUFFER_STRUCT(FLightAttenParameters)
    // IMPLEMENT_UNIFORM_BUFFER_STRUCT(FShadowMappingParameters)
    // IMPLEMENT_UNIFORM_BUFFER_STRUCT(FLightShaderParameters)

    ULightComponent::ULightComponent()
        : LightType(ELightType::Directional)
        , LightIntensity(FVector3f(1.474000, 1.850400, 1.911980)*10.f)
        , LightDistAttenuation(MakeDefaultInverseSquareAttenCurve())
        , LightAngleAttenuation(MakeDefaultInverseSquareAttenCurve())
        , bCastShadow(true)
        , ShadowMapResolution(FIntVector2(2048))
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
            [this](RenderGraph&) 
            {
                SceneProxy->SetPositionAndDirection(GetComponentLocation(), GetForwardVector(), GetUpVector());
            });

        USceneComponent::SendRenderTransform();
    }

    void ULightComponent::SendRenderDynamicData()
    {
        ENQUEUE_RENDER_COMMAND(ULightComponent_SendRenderDynamicData)(
            [this](RenderGraph&) 
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
        std::string ComponentName = InComponent->GetName();
        ENQUEUE_RENDER_COMMAND(FLightSceneProxy_Ctor)([this, ComponentName](RenderGraph& Graph)
        {
            LightParams = Graph.CreatePooledParameterBlock<shader::Light>(NFormat("shader::Light of {}", ComponentName));
            UpdateUniformBuffer(Graph);
        });
    }

    FLightSceneProxy::~FLightSceneProxy()
    {
        ENQUEUE_RENDER_COMMAND(FLightSceneProxy_Dtor)([this](RenderGraph&)
        {
            LightParams = nullptr;
        });
    }

    void FLightSceneProxy::SetPositionAndDirection(const FVector &InPosition, const FVector3f &InDirection, const FVector3f &InUp)
    {
        Position = InPosition;
        Direction = InDirection;
        Up = InUp;
    }

    void FLightSceneProxy::SetLightIntensity(const FVector3f& InLightIntensity)
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

    void FLightSceneProxy::SetShadowMapResolution(FIntVector2 InShadowMapResolution)
    {
        ShadowMapResolution = InShadowMapResolution;
        ScreenAspect = (float)ShadowMapResolution.x / (float)ShadowMapResolution.y;
    }

    void FLightSceneProxy::SetLightDistAttenParams(const FLightAttenuationCurve &AttenCurveParam) 
    { 
        DistAttenCurve = AttenCurveParam;
    }

    void FLightSceneProxy::SetLightAngleAttenParams(const FLightAttenuationCurve &AttenCurveParam)
    {
        AngleAttenCurve = AttenCurveParam;
    }

    void FLightSceneProxy::UpdateUniformBuffer(RenderGraph& Graph)
    {
        LightParams->distAttenCurve = DistAttenCurve;
        LightParams->angleAttenCurve = AngleAttenCurve;
        LightParams->position = FVector3f(Position);
        LightParams->intensity = LightIntensity;
        LightParams->direction = Direction;
        LightParams->castShadow = bCastShadow;
        LightParams->type = LightType;
        Graph.UpdateParameterBlock(LightParams.GetReference());
    }
}
