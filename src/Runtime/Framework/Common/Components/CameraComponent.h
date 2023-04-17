#pragma once
#include "SceneComponent.h"
#include "SceneView.h"
#include "Templates/ObjectMacros.h"
#include "RenderingThread.h"

namespace nilou {

    UCLASS()
    class UCameraComponent : public USceneComponent
    {
        friend class FScene;
        friend class FCameraSceneProxy;
        GENERATE_CLASS_INFO()
    public:

        UCameraComponent(AActor *InOwner=nullptr);

        virtual void OnRegister() override;
        virtual void OnUnregister() override;

        float GetAspectRatio() const { return (float)ScreenResolution.x / (float)ScreenResolution.y; }

		TUniformBufferRef<FViewShaderParameters> ViewUniformBuffer;

        /** Field of view. in radians. It will be omitted if the projection mode is Orthographic */
        float VerticalFieldOfView = glm::radians(50.f);

	    /** The desired width (in world units) of the orthographic view (ignored in Perspective mode) */
        float OrthoWidth = 25.6;

        float NearClipDistance;

        float FarClipDistance;

        ECameraProjectionMode ProjectionMode = ECameraProjectionMode::Perspective;
        
        glm::ivec2 ScreenResolution;

    };
}