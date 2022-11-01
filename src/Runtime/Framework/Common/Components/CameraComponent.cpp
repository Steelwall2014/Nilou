// #include <glm/gtc/matrix_transform.hpp>
#include "CameraComponent.h"
#include "Common/World.h"


namespace nilou {

    // IMPLEMENT_UNIFORM_BUFFER_STRUCT(FViewShaderParameters)

    FCameraParameters::FCameraParameters()
        : FOVY(50)
        , NearClipDistance(0.1)
        , FarClipDistance(10000)
        , AspectRatio(1.f)
        , ScreenResolution(glm::ivec2(1024, 1024))
        // , CameraType(ECameraType::CT_Perspective)
    {

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
                SceneProxy->SetCameraResolution(CameraParameters.ScreenResolution);
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
            return glm::perspective(glm::radians(CameraParameters.FOVY), CameraParameters.AspectRatio, CameraParameters.NearClipDistance, CameraParameters.FarClipDistance);
        // else if (CameraParameters.CameraType == ECameraType::CT_Ortho) 
        //     return glm::ortho(CameraParameters.FOVY, CameraParameters.AspectRatio, CameraParameters.NearClipDistance, CameraParameters.FarClipDistance);
    }

    FCameraSceneProxy *UCameraComponent::CreateSceneProxy()
    {
        return new FCameraSceneProxy(this);
    }

    void UCameraComponent::SetFieldOfView(float fovy)
    {
        CameraParameters.FOVY = fovy;
        MarkRenderDynamicDataDirty();
    }

    void UCameraComponent::SetCameraResolution(const ivec2 &CameraResolution)
    {
        CameraParameters.ScreenResolution = CameraResolution;
        CameraParameters.AspectRatio = (float)CameraResolution.x / (float)CameraResolution.y;
        bCameraResolutionDirty = true;
        MarkRenderDynamicDataDirty();
    }



    FCameraSceneProxy::FCameraSceneProxy(UCameraComponent *InComponent)
        : /*CameraParameters(InComponent->CameraParameters)
        , */CameraSceneInfo(nullptr)
    {
        InComponent->SceneProxy = this;
        ViewUniformBufferRHI = CreateUniformBuffer<FViewShaderParameters>();
        SetPositionAndDirection(InComponent->GetComponentLocation(), InComponent->GetForwardVector());
        SetViewProjectionMatrix(InComponent->CalcWorldToViewMatrix(), InComponent->CalcViewToClipMatrix());
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
        auto view = glm::lookAt(vec3(-5, 0, 0), vec3(1, 0, 0), vec3(0, 0, 1));
        auto projection = glm::perspective(glm::radians(45.0f), 1920.f/1080.f, 0.01f, 10000.f);
        auto a = view * vec4(0, 0.5, 1, 1);
        auto a0 = view * vec4(0, 0, 0, 1);
        auto b = projection*view * vec4(0, 0.5, 1, 1);
        auto b0 = projection*view * vec4(0, 0, 0, 1);
        auto c = b / b.w;
        auto c0 = b0 / b0.w;
        auto d = InWorldToView * vec4(0, 0.5, 1, 1);
        auto d0 = InWorldToView * vec4(0, 0, 0, 1);
        auto e = WorldToClip * vec4(0, 0.5, 1, 1);
        auto e0 = WorldToClip * vec4(0, 0, 0, 1);
        SceneView.ViewMatrix = InWorldToView;
        SceneView.ProjectionMatrix = InViewToClip;
        SceneView.ViewFrustum = FViewFrustum(InWorldToView, InViewToClip);
        if (CameraSceneInfo)
            CameraSceneInfo->SetNeedsUniformBufferUpdate(true);
    }

    void FCameraSceneProxy::SetCameraResolution(const ivec2 &InCameraResolution)
    {
        ScreenResolution = InCameraResolution;
        if (CameraSceneInfo)
            CameraSceneInfo->SetNeedsFramebufferUpdate(true);
    }
}