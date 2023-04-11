#include "SceneCaptureComponent.h"
#include "Common/Actor/Actor.h"
#include "TextureRenderTarget.h"
#include "DefferedShadingSceneRenderer.h"

namespace nilou {

    static std::vector<USceneCaptureComponent*> SceneCapturesToUpdate;

    void USceneCaptureComponent::HideComponent(UPrimitiveComponent* InComponent)
    {
        if (InComponent && InComponent->IsValid())
        {
            HiddenComponents.insert(InComponent);
        }
    }

    void USceneCaptureComponent::HideActorComponents(AActor* InActor)
    {
        if (InActor && InActor->IsValid())
        {
            auto Actor = InActor;
            std::vector<std::weak_ptr<UPrimitiveComponent>> PrimitiveComponents;
            Actor->GetComponents(PrimitiveComponents);
            for (auto WeakPrimComp : PrimitiveComponents)
            {
                HiddenComponents.insert(WeakPrimComp.lock().get());
            }
        }
    }

    void USceneCaptureComponent::ShowOnlyComponent(UPrimitiveComponent* InComponent)
    {
        if (InComponent && InComponent->IsValid())
        {
            ShowOnlyComponents.insert(InComponent);
        }
    }

    void USceneCaptureComponent::ShowOnlyActorComponents(AActor* InActor)
    {
        if (InActor && InActor->IsValid())
        {
            auto Actor = InActor;
            std::vector<std::weak_ptr<UPrimitiveComponent>> PrimitiveComponents;
            Actor->GetComponents(PrimitiveComponents);
            for (auto WeakPrimComp : PrimitiveComponents)
            {
                ShowOnlyComponents.insert(WeakPrimComp.lock().get());
            }
        }
    }

    void USceneCaptureComponent::CaptureScene()
    {
        UWorld* World = GetWorld();
        if (World && World->Scene)
        {
            // We must push any deferred render state recreations before causing any rendering to happen, to make sure that deleted resource references are updated
            World->SendAllEndOfFrameUpdates();
            UpdateSceneCaptureContents(World->Scene);
        }	
    }

    void USceneCaptureComponent::UpdateDeferredCaptures(FScene* Scene)
    {
        // Some trick, the first frame actually doesn't render anything.
        if (Scene->GetFrameNumber() <= 1)
            return;
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
        ViewFamily.HiddenComponents = HiddenComponents; 
        ViewFamily.ShowOnlyComponents = ShowOnlyComponents; 

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

    void USceneCaptureComponentCube::CaptureSceneDeferred()
    {
        SceneCapturesToUpdate.push_back(this);
    }

    void USceneCaptureComponentCube::TickComponent(double DeltaTime)
    {
        if (bCaptureEveryFrame)
        {
            CaptureSceneDeferred();
        }
    }

    void USceneCaptureComponentCube::SendRenderTransform()
    {
        
        if (bCaptureOnMovement && !bCaptureEveryFrame)
        {
            CaptureSceneDeferred();
        }

	    USceneCaptureComponent::SendRenderTransform();
    }

    void USceneCaptureComponentCube::UpdateSceneCaptureContents(FScene* Scene)
    {
        UpdateSceneCaptureContents_Internal(Scene, GetComponentLocation());
    }

    void USceneCaptureComponentCube::UpdateSceneCaptureContents_Internal(FScene* Scene, dvec3 Position)
    {
        if (TextureTarget == nullptr || TextureTarget->GetRenderTargetResource() == nullptr)
            return;
        FViewport Viewport;
        Viewport.Width = TextureTarget->GetSizeX();
        Viewport.Height = TextureTarget->GetSizeY();
        Viewport.RenderTarget = TextureTarget->GetRenderTargetResource();
        FSceneViewFamily ViewFamily(Viewport, Scene);    
        ViewFamily.HiddenComponents = HiddenComponents; 
        ViewFamily.ShowOnlyComponents = ShowOnlyComponents;  

        std::array<dvec3, 6> ForwardVectors = {
            dvec3(1, 0, 0), 
            dvec3(-1, 0, 0), 
            dvec3(0, 0, 1), 
            dvec3(0, 0, -1), 
            dvec3(0, 1, 0), 
            dvec3(0, -1, 0), 
        };
        std::array<dvec3, 6> UpVectors = {
            dvec3(0, 0, -1), 
            dvec3(0, 0, -1), 
            dvec3(0, 1, 0), 
            dvec3(0, -1, 0), 
            dvec3(0, 0, -1), 
            dvec3(0, 0, -1), 
        };

        std::vector<FSceneView> SceneViews;
        SceneViews.reserve(ViewUniformBuffers.size());
        for (int i = 0; i < ViewUniformBuffers.size(); i++)
        {
            SceneViews.emplace_back(
                glm::radians(90.0), 
                0.1, 30000, 
                Position, 
                ForwardVectors[i], 
                UpVectors[i],
                ivec2(Viewport.Width, Viewport.Height), 
                ViewUniformBuffers[i]);  
            ViewFamily.Views.push_back(&SceneViews[i]); 
        }  

        FSceneRenderer* SceneRenderer = FSceneRenderer::CreateSceneRenderer(&ViewFamily);

        ENQUEUE_RENDER_COMMAND(USceneCaptureComponentCube_UpdateSceneCaptureContents)(
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

    void USceneCaptureComponentCube::OnRegister()
    {
        for (int i = 0; i < ViewUniformBuffers.size(); i++)
        {
            ViewUniformBuffers[i] = CreateUniformBuffer<FViewShaderParameters>();
            BeginInitResource(ViewUniformBuffers[i].get());
        }
    }

    void USceneCaptureComponentCube::OnUnregister()
    {
        for (int i = 0; i < ViewUniformBuffers.size(); i++)
        {
            auto ToDelete = ViewUniformBuffers[i];
            ENQUEUE_RENDER_COMMAND(USceneCaptureComponentCube_OnUnregister)(
                [ToDelete](FDynamicRHI*) 
                {
                    ToDelete->ReleaseResource();
                });
            ViewUniformBuffers[i] = nullptr;
        }
    }

}