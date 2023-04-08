#pragma once
#include "PrimitiveComponent.h"

namespace nilou {

    UCLASS()
    class USceneCaptureComponent : public USceneComponent
    {
        GENERATE_CLASS_INFO()
    public:
        USceneCaptureComponent(AActor *InOwner = nullptr) 
            : USceneComponent(InOwner)
            , bCaptureEveryFrame(true)
            , bCaptureOnMovement(true)
        { }

        bool bCaptureEveryFrame;

	    bool bCaptureOnMovement;

        std::vector<std::weak_ptr<UPrimitiveComponent>> HiddenComponents;
        
        void HideComponent(std::weak_ptr<UPrimitiveComponent> InComponent);

        void HideActorComponents(std::weak_ptr<AActor> InActor);

        static void USceneCaptureComponent::UpdateDeferredCaptures(FScene* Scene);
    
    protected:

	    virtual void UpdateSceneCaptureContents(FScene* Scene) {};

    };

    UCLASS()
    class USceneCaptureComponent2D : public USceneCaptureComponent
    {
        GENERATE_CLASS_INFO()
    public:
        USceneCaptureComponent2D(AActor *InOwner = nullptr) 
            : USceneCaptureComponent(InOwner)
        { }

        /** Field of view. in radians */
        float VerticalFieldOfView;

	    /** The desired width (in world units) of the orthographic view (ignored in Perspective mode) */
        float OrthoWidth;

        class UTextureRenderTarget2D* TextureTarget;

        /** Render the scene to the texture the next time the main view is rendered. */
        void CaptureSceneDeferred();

        // For backwards compatibility
        void UpdateContent() { CaptureSceneDeferred(); }

        virtual void UpdateSceneCaptureContents(FScene* Scene) override;

        virtual void TickComponent(double DeltaTime) override;

        virtual void SendRenderTransform() override;

        virtual void OnRegister() override;

        virtual void OnUnregister() override;

    protected:

        TUniformBufferRef<FViewShaderParameters> ViewUniformBuffer;

    };

    UCLASS()
    class USceneCaptureComponentCube : public USceneCaptureComponent
    {
        GENERATE_CLASS_INFO()
    public:
        USceneCaptureComponentCube(AActor *InOwner = nullptr) 
            : USceneCaptureComponent(InOwner)
        { }

        std::weak_ptr<class UTextureRenderTargetCube> TextureTarget;
    };

}