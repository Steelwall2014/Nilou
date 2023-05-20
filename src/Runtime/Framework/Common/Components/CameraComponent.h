#pragma once
#include "SceneComponent.h"
#include "SceneView.h"
#include "Templates/ObjectMacros.h"
#include "RenderingThread.h"

namespace nilou {

    class NCLASS UCameraComponent : public USceneComponent
    {
        friend class FScene;
        friend class FCameraSceneProxy;
        GENERATED_BODY()
    public:

        UCameraComponent();

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
        
        ivec2 ScreenResolution;

    };
}