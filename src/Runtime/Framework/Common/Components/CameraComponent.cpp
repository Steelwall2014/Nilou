// #include <glm/gtc/matrix_transform.hpp>
#include "CameraComponent.h"
#include "Common/World.h"


namespace nilou {

    UCameraComponent::UCameraComponent() 
        : VerticalFieldOfView(glm::radians(50.f))
        , NearClipDistance(0.1)
        , FarClipDistance(30000)
        , ScreenResolution(ivec2(1024, 1024))
    { 
    }

    void UCameraComponent::OnRegister()
    {
        USceneComponent::OnRegister();

        ENQUEUE_RENDER_COMMAND(ACameraActor_Cons)(
            [this](RHICommandList&)
            {
                ViewUniformBuffer = RenderGraph::CreateExternalUniformBuffer<FViewShaderParameters>("");
            });
    }

    void UCameraComponent::OnUnregister()
    {
        ENQUEUE_RENDER_COMMAND(ACameraActor_Cons)(
            [this](RHICommandList&)
            {
                ViewUniformBuffer = nullptr;
            });
        USceneComponent::OnUnregister();
    }

}