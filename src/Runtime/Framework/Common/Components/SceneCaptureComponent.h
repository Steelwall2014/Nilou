#pragma once
#include "PrimitiveComponent.h"
#include "Common/EngineTypes.h"

namespace nilou {

    class NCLASS USceneCaptureComponent : public USceneComponent
    {
        GENERATE_BODY()
    public:
        USceneCaptureComponent() 
            : bCaptureEveryFrame(true)
            , bCaptureOnMovement(true)
        { }

        bool bCaptureEveryFrame;

	    bool bCaptureOnMovement;

        ESceneCaptureSource CaptureSource = SCS_LinearColor;

        std::set<UPrimitiveComponent*> HiddenComponents;

        std::set<UPrimitiveComponent*> ShowOnlyComponents;
        
        void HideComponent(UPrimitiveComponent* InComponent);

        void HideActorComponents(AActor* InActor);
        
        void ShowOnlyComponent(UPrimitiveComponent* InComponent);

        void ShowOnlyActorComponents(AActor* InActor);

        void CaptureScene();

        static void USceneCaptureComponent::UpdateDeferredCaptures(FScene* Scene);
    
    protected:

	    virtual void UpdateSceneCaptureContents(FScene* Scene) {};

    };

    class NCLASS USceneCaptureComponent2D : public USceneCaptureComponent
    {
        GENERATE_BODY()
    public:
        USceneCaptureComponent2D() 
        { }

        /** Field of view. in radians */
        float VerticalFieldOfView = glm::radians(50.f);

	    /** The desired width (in world units) of the orthographic view (ignored in Perspective mode) */
        float OrthoWidth = 25.6;

        ECameraProjectionMode ProjectionMode = ECameraProjectionMode::Perspective;

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

    class NCLASS USceneCaptureComponentCube : public USceneCaptureComponent
    {
        GENERATE_BODY()
    public:
        USceneCaptureComponentCube() 
            : TextureTarget(nullptr)
        { }

        class UTextureRenderTargetCube* TextureTarget;

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

        std::array<TUniformBufferRef<FViewShaderParameters>, 6> ViewUniformBuffers;

        void UpdateSceneCaptureContents_Internal(FScene* Scene, dvec3 Position);
    };

}