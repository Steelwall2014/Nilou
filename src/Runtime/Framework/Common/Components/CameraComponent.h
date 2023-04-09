#pragma once
#include "SceneComponent.h"
#include "SceneView.h"
#include "Templates/ObjectMacros.h"
#include "RenderingThread.h"

namespace nilou {

    // struct FCameraParameters
    // {
    //     // EViewType CameraType;

    //     FCameraParameters();
    // };

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

        float GetAspectRatio() const { return AspectRatio; }

        float GetNearClipDistance() const { return NearClipDistance; }

        float GetFarClipDistance() const { return FarClipDistance; }

        float GetFieldOfView() const { return VerticalFieldOfView; }
        /**
         * Set the vertical field of view. 
         * 
         * @param InVerticalFieldOfView The angle of vertical FOV, in radians
         */
        void SetFieldOfView(float InVerticalFieldOfView);

        ivec2 GetCameraResolution() const { return ScreenResolution; }

        void SetCameraResolution(const ivec2 &CameraResolution);

		TUniformBufferRef<FViewShaderParameters> ViewUniformBuffer;

    protected:

        /** Field of view. in radians */
        float VerticalFieldOfView;
        float NearClipDistance;
        float FarClipDistance;

        /** AspectRatio = ScreenWidth / ScreenHeight */
        float AspectRatio;
        glm::ivec2 ScreenResolution;
    };
}