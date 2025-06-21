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
            [this](RenderGraph&)
            {
                ViewUniformBuffer = RenderGraph::CreateExternalUniformBuffer<FViewShaderParameters>("ViewUniformBuffer", nullptr);
            });
    }

    void UCameraComponent::OnUnregister()
    {
        ENQUEUE_RENDER_COMMAND(ACameraActor_Cons)(
            [this](RenderGraph&)
            {
                ViewUniformBuffer = nullptr;
            });
        USceneComponent::OnUnregister();
    }

}