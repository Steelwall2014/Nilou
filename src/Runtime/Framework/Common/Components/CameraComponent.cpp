// #include <glm/gtc/matrix_transform.hpp>
#include "CameraComponent.h"
#include "Common/World.h"


namespace nilou {

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
            SceneProxy->SetPositionAndDirection(GetComponentLocation(), GetForwardVector());
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
            return glm::perspective(glm::radians(FOVY), AspectRatio, NearClipDistance, FarClipDistance);
        // else if (CameraParameters.CameraType == ECameraType::CT_Ortho) 
        //     return glm::ortho(CameraParameters.FOVY, CameraParameters.AspectRatio, CameraParameters.NearClipDistance, CameraParameters.FarClipDistance);
    }

    FCameraSceneProxy *UCameraComponent::CreateSceneProxy()
    {
        return new FCameraSceneProxy(this);
    }

    void UCameraComponent::SetFieldOfView(float fovy)
    {
        FOVY = fovy;
        MarkRenderDynamicDataDirty();
    }

    void UCameraComponent::SetCameraResolution(const ivec2 &CameraResolution)
    {
        ScreenResolution = CameraResolution;
        AspectRatio = (float)CameraResolution.x / (float)CameraResolution.y;
        bCameraResolutionDirty = true;
        MarkRenderDynamicDataDirty();
    }



    FCameraSceneProxy::FCameraSceneProxy(UCameraComponent *InComponent)
        : CameraSceneInfo(nullptr)
    {
        InComponent->SceneProxy = this;
        ViewUniformBufferRHI = CreateUniformBuffer<FViewShaderParameters>();
        SetPositionAndDirection(InComponent->GetComponentLocation(), InComponent->GetForwardVector());
        SetViewProjectionMatrix(InComponent->CalcWorldToViewMatrix(), InComponent->CalcViewToClipMatrix());
        SetCameraResolution(InComponent->ScreenResolution);
        // ViewUniformBufferRHI->InitRHI();
        BeginInitResource(ViewUniformBufferRHI.get());
        if (CameraSceneInfo)
            CameraSceneInfo->SetNeedsUniformBufferUpdate(false);
    }

    void FCameraSceneProxy::SetPositionAndDirection(const glm::vec3 &InPosition, const glm::vec3 &InDirection)
    {
        ViewUniformBufferRHI->Data.CameraPosition = InPosition;
        ViewUniformBufferRHI->Data.CameraDirection = InDirection;
        if (CameraSceneInfo)
            CameraSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FCameraSceneProxy::SetViewProjectionMatrix(const glm::mat4 &InWorldToView, const glm::mat4 &InViewToClip)
    {
        glm::mat4 WorldToClip = InViewToClip * InWorldToView;
        ViewUniformBufferRHI->Data.WorldToView = InWorldToView;
        ViewUniformBufferRHI->Data.ViewToClip = InViewToClip;
        ViewUniformBufferRHI->Data.WorldToClip = WorldToClip;
        ViewUniformBufferRHI->Data.ClipToWorld = glm::inverse(WorldToClip);
        SceneView.ViewMatrix = InWorldToView;
        SceneView.ProjectionMatrix = InViewToClip;
        SceneView.ViewFrustum = FViewFrustum(InWorldToView, InViewToClip);
        if (CameraSceneInfo)
            CameraSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FCameraSceneProxy::SetCameraResolution(const ivec2 &InCameraResolution)
    {
        SceneView.ScreenResolution = InCameraResolution;
        ViewUniformBufferRHI->Data.CameraResolution = InCameraResolution;
        if (CameraSceneInfo)
            CameraSceneInfo->SetNeedsFramebufferUpdate(true);
    }

    void FCameraSceneProxy::SetCameraClipDistances(float InCameraNearClipDist, float InCameraFarClipDist)
    {
        ViewUniformBufferRHI->Data.CameraNearClipDist = InCameraNearClipDist;
        ViewUniformBufferRHI->Data.CameraFarClipDist = InCameraFarClipDist;
        if (CameraSceneInfo)
            CameraSceneInfo->SetNeedsFramebufferUpdate(true);
    }
}