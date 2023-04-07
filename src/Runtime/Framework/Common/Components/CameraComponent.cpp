// #include <glm/gtc/matrix_transform.hpp>
#include "CameraComponent.h"
#include "Common/World.h"


namespace nilou {

    UCameraComponent::UCameraComponent(AActor *InOwner) 
        : USceneComponent(InOwner)
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

        ViewUniformBuffer = CreateUniformBuffer<FViewShaderParameters>();
        ENQUEUE_RENDER_COMMAND(ACameraActor_Cons)(
            [this](FDynamicRHI*)
            {
                ViewUniformBuffer->InitResource();
            });
    }

    void UCameraComponent::OnUnregister()
    {
        ViewUniformBuffer = nullptr;
        USceneComponent::OnUnregister();
    }

    void UCameraComponent::SetFieldOfView(float InVerticalFieldOfView)
    {
        if (VerticalFieldOfView != InVerticalFieldOfView)
        {
            VerticalFieldOfView = InVerticalFieldOfView;
        }
    }

    void UCameraComponent::SetCameraResolution(const ivec2 &CameraResolution)
    {
        if (ScreenResolution != CameraResolution)
        {
            ScreenResolution = CameraResolution;
            AspectRatio = (float)CameraResolution.x / (float)CameraResolution.y;
        }
    }

}