// #include <glm/gtc/matrix_transform.hpp>
#include "CameraComponent.h"
#include "Common/World.h"


namespace nilou {

    UCameraComponent::UCameraComponent(AActor *InOwner) 
        : USceneComponent(InOwner)
        , SceneProxy(nullptr)
        , VerticalFieldOfView(glm::radians(50.f))
        , NearClipDistance(0.1)
        , FarClipDistance(30000)
        , AspectRatio(1.f)
        , ScreenResolution(glm::ivec2(1024, 1024))
    { 
    }

    void UCameraComponent::OnRegister()
    {
        USceneComponent::OnRegister();
        UWorld *World = GetWorld();
        if (World)
        {
            if (GetOwner())
            {
                World->CameraComponents.push_back(this);
                if (World->MainCameraComponent == nullptr)
                    World->MainCameraComponent = this;
            }
        }
    }

    void UCameraComponent::OnUnregister()
    {
        UWorld *World = GetWorld();
        if (World)
        {
            if (GetOwner())
            {
                for (auto iter = World->CameraComponents.begin(); iter != World->CameraComponents.end(); iter++)
                {
                    if (*iter == this)
                    {
                        World->CameraComponents.erase(iter);
                        break;
                    }
                }
            }
        }
        USceneComponent::OnUnregister();
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

    glm::dmat4 UCameraComponent::CalcWorldToViewMatrix()
    {
        glm::dvec3 forward = GetForwardVector();
        glm::dvec3 location = GetComponentLocation();
        glm::dvec3 up = GetUpVector();
        return glm::lookAt(location, location+forward, up);
    }

    glm::mat4 UCameraComponent::CalcViewToClipMatrix()
    {
        // if (CameraParameters.CameraType == EViewType::CT_Perspective)
            return glm::perspective(VerticalFieldOfView, AspectRatio, NearClipDistance, FarClipDistance);
        // else if (CameraParameters.CameraType == EViewType::CT_Ortho) 
        //     return glm::ortho(CameraParameters.VerticalFieldOfView, CameraParameters.AspectRatio, CameraParameters.NearClipDistance, CameraParameters.FarClipDistance);
    }

    FCameraSceneProxy *UCameraComponent::CreateSceneProxy()
    {
        return new FCameraSceneProxy(this);
    }

    void UCameraComponent::SetFieldOfView(float InVerticalFieldOfView)
    {
        if (VerticalFieldOfView != InVerticalFieldOfView)
        {
            VerticalFieldOfView = InVerticalFieldOfView;
            MarkRenderStateDirty();
        }
    }

    void UCameraComponent::SetCameraResolution(const ivec2 &CameraResolution)
    {
        if (ScreenResolution != CameraResolution)
        {
            ScreenResolution = CameraResolution;
            AspectRatio = (float)CameraResolution.x / (float)CameraResolution.y;
            MarkRenderStateDirty();
        }
    }

    FViewFrustum UCameraComponent::CalcViewFrustum()
    {
        return FViewFrustum(
            GetComponentLocation(), 
            GetForwardVector(), 
            GetUpVector(), AspectRatio, VerticalFieldOfView, NearClipDistance, FarClipDistance);
    }

    void UCameraComponent::SetMaxCascadeShadowMapDistance(double MaxCSMDistance)
    {
        if (MaxCSMDistance != MaxCascadeShadowMapDistance)
        {
            MaxCascadeShadowMapDistance = MaxCSMDistance;
            MarkRenderStateDirty();
        }
    }



    FCameraSceneProxy::FCameraSceneProxy(UCameraComponent *InComponent)
        : ViewSceneInfo(nullptr)
    {
        InComponent->SceneProxy = this;
        MaxCascadeShadowMapDistance = InComponent->MaxCascadeShadowMapDistance;
        ViewUniformBufferRHI = CreateUniformBuffer<FViewShaderParameters>();
        SetPositionAndDirection(InComponent->GetComponentLocation(), InComponent->GetForwardVector(), InComponent->GetUpVector());
        SetViewProjectionMatrix(InComponent->CalcWorldToViewMatrix(), InComponent->CalcViewToClipMatrix());
        SetCameraResolution(InComponent->ScreenResolution);
        SetCameraClipDistances(InComponent->NearClipDistance, InComponent->FarClipDistance);
        SetFieldOfView(InComponent->GetFieldOfView());
        BeginInitResource(ViewUniformBufferRHI.get());
        if (ViewSceneInfo)
        {
            ViewSceneInfo->SetNeedsFramebufferUpdate(true);
            ViewSceneInfo->SetNeedsUniformBufferUpdate(true);
        }
    }

    void FCameraSceneProxy::SetPositionAndDirection(const glm::dvec3 &InPosition, const glm::vec3 &InDirection, const glm::vec3 &InUp)
    {
        Position = InPosition;
        Forward = InDirection;
        Up = InUp;
        ViewUniformBufferRHI->Data.CameraPosition = InPosition;
        ViewUniformBufferRHI->Data.CameraDirection = InDirection;
        UpdateFrustum();
        if (ViewSceneInfo)
            ViewSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FCameraSceneProxy::SetViewProjectionMatrix(const glm::dmat4 &InWorldToView, const glm::mat4 &InViewToClip)
    {
        ViewMatrix = InWorldToView;
        ProjectionMatrix = InViewToClip;

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
        ScreenResolution = InCameraResolution;
        AspectRatio = (float)InCameraResolution.x / (float)InCameraResolution.y;
        ViewUniformBufferRHI->Data.CameraResolution = InCameraResolution;
        UpdateFrustum();
        if (ViewSceneInfo)
        {
            ViewSceneInfo->SetNeedsFramebufferUpdate(true);
            ViewSceneInfo->SetNeedsUniformBufferUpdate(true);
        }
    }

    void FCameraSceneProxy::SetCameraClipDistances(float InCameraNearClipDist, float InCameraFarClipDist)
    {
        NearClipDistance = InCameraNearClipDist;
        FarClipDistance = InCameraFarClipDist;
        ViewUniformBufferRHI->Data.CameraNearClipDist = InCameraNearClipDist;
        ViewUniformBufferRHI->Data.CameraFarClipDist = InCameraFarClipDist;
        UpdateFrustum();
        if (ViewSceneInfo)
        {
            ViewSceneInfo->SetNeedsFramebufferUpdate(true);
            ViewSceneInfo->SetNeedsUniformBufferUpdate(true);
        }
    }

    void FCameraSceneProxy::SetFieldOfView(float InVerticalFieldOfView)
    {
        VerticalFieldOfView = InVerticalFieldOfView;
        ViewUniformBufferRHI->Data.CameraVerticalFieldOfView = InVerticalFieldOfView;
        UpdateFrustum();
        if (ViewSceneInfo)
        {
            ViewSceneInfo->SetNeedsFramebufferUpdate(true);
            ViewSceneInfo->SetNeedsUniformBufferUpdate(true);
        }
    }

    FSceneView FCameraSceneProxy::GetSceneView()
    {
        FSceneView SceneView;
        SceneView.ViewFrustum = ViewFrustum;
        SceneView.ProjectionMatrix = ProjectionMatrix;
        SceneView.ViewMatrix = ViewMatrix;
        SceneView.Position = Position;
        SceneView.Forward = Forward;
        SceneView.Up = Up;
        SceneView.AspectRatio = AspectRatio;
        SceneView.VerticalFieldOfView = VerticalFieldOfView;
        SceneView.NearClipDistance = NearClipDistance;
        SceneView.FarClipDistance = FarClipDistance;
        SceneView.ScreenResolution = ScreenResolution;
        SceneView.ViewType = ViewType;
        SceneView.ViewInfo = ViewSceneInfo;
        return SceneView;
    }

    void FCameraSceneProxy::UpdateFrustum()
    {
        ViewFrustum = FViewFrustum(
            Position, Forward, Up, 
            AspectRatio, VerticalFieldOfView, 
            NearClipDistance, FarClipDistance);
        for (int i = 0; i < 6; i++)
            ViewUniformBufferRHI->Data.FrustumPlanes[i] = dvec4(ViewFrustum.Planes[i].Normal, ViewFrustum.Planes[i].Distance);

        // if (ViewSceneInfo)
        //     ViewSceneInfo->SetNeedsUniformBufferUpdate(true);
    }


}