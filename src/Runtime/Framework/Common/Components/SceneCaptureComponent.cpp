#include "SceneCaptureComponent.h"
#include "Common/Actor/Actor.h"
#include "TextureRenderTarget.h"
#include "DefferedShadingSceneRenderer.h"

namespace nilou {

    static std::vector<USceneCaptureComponent*> SceneCapturesToUpdate;

    void USceneCaptureComponent::HideComponent(std::weak_ptr<UPrimitiveComponent> InComponent)
    {
        if (!InComponent.expired())
        {
            HiddenComponents.push_back(InComponent);
        }
    }

    void USceneCaptureComponent::HideActorComponents(std::weak_ptr<AActor> InActor)
    {
        if (!InActor.expired())
        {
            auto Actor = InActor.lock();
            std::vector<std::weak_ptr<UPrimitiveComponent>> PrimitiveComponents;
            Actor->GetComponents(PrimitiveComponents);
            for (auto WeakPrimComp : PrimitiveComponents)
            {
                HiddenComponents.push_back(WeakPrimComp);
            }
        }
    }

    void USceneCaptureComponent::UpdateDeferredCaptures(FScene* Scene)
    {
        for (USceneCaptureComponent* Component : SceneCapturesToUpdate)
        {
            if (Component && Component->IsValid())
                Component->UpdateSceneCaptureContents(Scene);
        }
        SceneCapturesToUpdate.clear();
    }

    void USceneCaptureComponent2D::CaptureSceneDeferred()
    {
        SceneCapturesToUpdate.push_back(this);
    }

    void USceneCaptureComponent2D::TickComponent(double DeltaTime)
    {
        if (bCaptureEveryFrame)
        {
            CaptureSceneDeferred();
        }
    }

    void USceneCaptureComponent2D::SendRenderTransform()
    {
        
        if (bCaptureOnMovement && !bCaptureEveryFrame)
        {
            CaptureSceneDeferred();
        }

	    USceneCaptureComponent::SendRenderTransform();
    }

    void USceneCaptureComponent2D::UpdateSceneCaptureContents(FScene* Scene)
    {
        if (TextureTarget->GetRenderTargetResource() == nullptr)
            return;
        FViewport Viewport;
        Viewport.Width = TextureTarget->GetSizeX();
        Viewport.Height = TextureTarget->GetSizeY();
        Viewport.RenderTarget = TextureTarget->GetRenderTargetResource();
        FSceneViewFamily ViewFamily(Viewport, Scene);   

        FSceneView SceneView(
            VerticalFieldOfView, 
            0.1, 30000, 
            GetComponentLocation(), 
            GetForwardVector(), 
            GetUpVector(),
            ivec2(Viewport.Width, Viewport.Height), 
            ViewUniformBuffer);  
        ViewFamily.Views.push_back(&SceneView);   

        FSceneRenderer* SceneRenderer = FSceneRenderer::CreateSceneRenderer(&ViewFamily);

        ENQUEUE_RENDER_COMMAND(USceneCaptureComponent2D_UpdateSceneCaptureContents)(
            [SceneRenderer](FDynamicRHI*) 
            {
                for (FViewInfo& View : SceneRenderer->Views)
                {
                    View.ViewUniformBuffer->UpdateUniformBuffer();
                }

                SceneRenderer->Render();
                
                delete SceneRenderer;
            });

    }

    void USceneCaptureComponent2D::OnRegister()
    {
        ViewUniformBuffer = CreateUniformBuffer<FViewShaderParameters>();
        BeginInitResource(ViewUniformBuffer.get());
    }

    void USceneCaptureComponent2D::OnUnregister()
    {
        auto ToDelete = ViewUniformBuffer;
        ENQUEUE_RENDER_COMMAND(USceneCaptureComponent2D_OnUnregister)(
            [ToDelete](FDynamicRHI*) 
            {
                ToDelete->ReleaseResource();
            });
        ViewUniformBuffer = nullptr;
    }

}