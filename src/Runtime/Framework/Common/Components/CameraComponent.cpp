// #include <glm/gtc/matrix_transform.hpp>
#include "CameraComponent.h"
#include "Common/World.h"


namespace nilou {

    void UCameraComponent::OnRegister()
    {
        UWorld *World = GetWorld();
        if (World)
        {
            if (GetOwner())
                World->CameraActors.insert(GetOwner());
        }
    }

    void UCameraComponent::OnUnregister()
    {
        UWorld *World = GetWorld();
        if (World)
        {
            if (GetOwner())
                World->CameraActors.erase(GetOwner());
        }
    }

    void UCameraComponent::CreateRenderState()
    {
        USceneComponent::CreateRenderState();
        UWorld* World = GetWorld();
        if(World && World->Scene)
        {
            World->Scene->AddCamera(this);
        }
    }

    void UCameraComponent::DestroyRenderState()
    {
        UWorld* World = GetWorld();
        if(World && World->Scene)
        {
            World->Scene->RemoveCamera(this);
        }

        USceneComponent::DestroyRenderState();
    }

    void UCameraComponent::SendRenderTransform()
    {
        if (SceneProxy)
        {
            SceneProxy->SetPositionAndDirection(GetComponentLocation(), GetForwardVector(), GetUpVector());
            SceneProxy->SetViewProjectionMatrix(CalcWorldToViewMatrix(), CalcViewToClipMatrix());
        }
        USceneComponent::SendRenderTransform();
    }

    void UCameraComponent::SendRenderDynamicData()
    {
        if (SceneProxy)
        {
            SceneProxy->SetViewProjectionMatrix(CalcWorldToViewMatrix(), CalcViewToClipMatrix());
            if (bCameraResolutionDirty)
            {
                SceneProxy->SetCameraResolution(ScreenResolution);
                bCameraResolutionDirty = false;
            }
        }

        USceneComponent::SendRenderDynamicData();
    }

    glm::mat4 UCameraComponent::CalcWorldToViewMatrix()
    {
        glm::vec3 forward = GetForwardVector();
        glm::vec3 location = GetComponentLocation();
        glm::vec3 up = GetUpVector();
        return glm::lookAt(location, location+forward, up);
    }

    glm::mat4 UCameraComponent::CalcViewToClipMatrix()
    {
        // if (CameraParameters.CameraType == ECameraType::CT_Perspective)
            return glm::perspective(VerticalFieldOfView, AspectRatio, NearClipDistance, FarClipDistance);
        // else if (CameraParameters.CameraType == ECameraType::CT_Ortho) 
        //     return glm::ortho(CameraParameters.VerticalFieldOfView, CameraParameters.AspectRatio, CameraParameters.NearClipDistance, CameraParameters.FarClipDistance);
    }

    FCameraSceneProxy *UCameraComponent::CreateSceneProxy()
    {
        return new FCameraSceneProxy(this);
    }

    void UCameraComponent::SetFieldOfView(float InVerticalFieldOfView)
    {
        VerticalFieldOfView = InVerticalFieldOfView;
        MarkRenderDynamicDataDirty();
    }

    void UCameraComponent::SetCameraResolution(const ivec2 &CameraResolution)
    {
        ScreenResolution = CameraResolution;
        AspectRatio = (float)CameraResolution.x / (float)CameraResolution.y;
        bCameraResolutionDirty = true;
        MarkRenderDynamicDataDirty();
    }

    FViewFrustum UCameraComponent::CalcViewFrustum()
    {
        return FViewFrustum(
            GetComponentLocation(), 
            GetForwardVector(), 
            GetUpVector(), AspectRatio, VerticalFieldOfView, NearClipDistance, FarClipDistance);
    }



    FCameraSceneProxy::FCameraSceneProxy(UCameraComponent *InComponent)
        : ViewSceneInfo(nullptr)
    {
        InComponent->SceneProxy = this;
        ViewUniformBufferRHI = CreateUniformBuffer<FViewShaderParameters>();
        SetPositionAndDirection(InComponent->GetComponentLocation(), InComponent->GetForwardVector(), InComponent->GetUpVector());
        SetViewProjectionMatrix(InComponent->CalcWorldToViewMatrix(), InComponent->CalcViewToClipMatrix());
        SetCameraResolution(InComponent->ScreenResolution);
        SetCameraClipDistances(InComponent->NearClipDistance, InComponent->FarClipDistance);
        BeginInitResource(ViewUniformBufferRHI.get());
        if (ViewSceneInfo)
            ViewSceneInfo->SetNeedsUniformBufferUpdate(false);
    }

    void FCameraSceneProxy::SetPositionAndDirection(const glm::dvec3 &InPosition, const glm::vec3 &InDirection, const glm::vec3 &InUp)
    {
        SceneView.Position = InPosition;
        SceneView.Forward = InDirection;
        SceneView.Up = InUp;
        ViewUniformBufferRHI->Data.CameraPosition = InPosition;
        ViewUniformBufferRHI->Data.CameraDirection = InDirection;
        if (ViewSceneInfo)
            ViewSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FCameraSceneProxy::SetViewProjectionMatrix(const glm::dmat4 &InWorldToView, const glm::mat4 &InViewToClip)
    {
        SceneView.ViewMatrix = InWorldToView;
        SceneView.ProjectionMatrix = InViewToClip;

        mat4 RelativeWorldToView = InWorldToView;
        RelativeWorldToView[3][0] = 0;
        RelativeWorldToView[3][1] = 0;
        RelativeWorldToView[3][2] = 0;
        ViewUniformBufferRHI->Data.RelWorldToView = RelativeWorldToView;
        ViewUniformBufferRHI->Data.ViewToClip = InViewToClip;
        ViewUniformBufferRHI->Data.RelWorldToClip = InViewToClip * RelativeWorldToView;
        ViewUniformBufferRHI->Data.AbsWorldToClip = InViewToClip * mat4(InWorldToView);
        ViewUniformBufferRHI->Data.ClipToView = glm::inverse(InViewToClip);
        // dmat4 AbsWorldToClip = dmat4(InViewToClip) * InWorldToView;
        ViewUniformBufferRHI->Data.RelClipToWorld = glm::inverse(InViewToClip * RelativeWorldToView);
        if (ViewSceneInfo)
            ViewSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FCameraSceneProxy::SetCameraResolution(const ivec2 &InCameraResolution)
    {
        SceneView.ScreenResolution = InCameraResolution;
        ViewUniformBufferRHI->Data.CameraResolution = InCameraResolution;
        if (ViewSceneInfo)
            ViewSceneInfo->SetNeedsFramebufferUpdate(true);
    }

    void FCameraSceneProxy::SetCameraClipDistances(float InCameraNearClipDist, float InCameraFarClipDist)
    {
        SceneView.NearClipDistance = InCameraNearClipDist;
        SceneView.FarClipDistance = InCameraFarClipDist;
        ViewUniformBufferRHI->Data.CameraNearClipDist = InCameraNearClipDist;
        ViewUniformBufferRHI->Data.CameraFarClipDist = InCameraFarClipDist;
        if (ViewSceneInfo)
            ViewSceneInfo->SetNeedsFramebufferUpdate(true);
    }

    const FSceneView &FCameraSceneProxy::GetSceneView()
    {
        return SceneView;
    }
}