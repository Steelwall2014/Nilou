#include "SceneCaptureComponent.h"
#include "Common/Actor/Actor.h"
#include "TextureRenderTarget.h"
#include "DeferredShadingSceneRenderer.h"
#include "Common/World.h"

namespace nilou {

    static std::vector<USceneCaptureComponent*> SceneCapturesToUpdate;

    void USceneCaptureComponent::HideComponent(UPrimitiveComponent* InComponent)
    {
        if (InComponent)
        {
            HiddenComponents.insert(InComponent);
        }
    }

    void USceneCaptureComponent::HideActorComponents(AActor* InActor)
    {
        if (InActor)
        {
            auto Actor = InActor;
            std::vector<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents(PrimitiveComponents);
            for (auto PrimComp : PrimitiveComponents)
            {
                HiddenComponents.insert(PrimComp);
            }
        }
    }

    void USceneCaptureComponent::ShowOnlyComponent(UPrimitiveComponent* InComponent)
    {
        if (InComponent)
        {
            ShowOnlyComponents.insert(InComponent);
        }
    }

    void USceneCaptureComponent::ShowOnlyActorComponents(AActor* InActor)
    {
        if (InActor)
        {
            auto Actor = InActor;
            std::vector<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents(PrimitiveComponents);
            for (auto PrimComp : PrimitiveComponents)
            {
                ShowOnlyComponents.insert(PrimComp);
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
        // For some reason, the first frame actually doesn't render anything.
        // It's a bug, but I don't want to fix it because I can't find where is the wrong.
        if (Scene->GetFrameNumber() <= 1)
            return;
        for (USceneCaptureComponent* Component : SceneCapturesToUpdate)
        {
            if (Component)
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
        ViewFamily.bIsSceneCapture = true;
        ViewFamily.CaptureSource = CaptureSource;
        if (CaptureSource == SCS_GammaColor)
        {
            ViewFamily.GammaCorrection = 2.2;
            ViewFamily.bEnableToneMapping = true;
        }

        FSceneView SceneView(
            ProjectionMode,
            VerticalFieldOfView, OrthoWidth, 
            0.1, 30000, 
            GetComponentLocation(), 
            GetForwardVector(), 
            GetUpVector(),
            ivec2(Viewport.Width, Viewport.Height));  
        ViewFamily.Views.push_back(SceneView);   

        FSceneRenderer* SceneRenderer = FSceneRenderer::CreateSceneRenderer(ViewFamily);

        ENQUEUE_RENDER_COMMAND(USceneCaptureComponent2D_UpdateSceneCaptureContents)(
            [SceneRenderer](RenderGraph&) 
            {
                // for (auto& View : SceneRenderer->Views)
                // {
                //     View.ViewUniformBuffer->UpdateUniformBuffer();
                // }
                RenderGraph& Graph = FRenderingThread::GetRenderGraph();
                SceneRenderer->Render(Graph);
                
                delete SceneRenderer;
            });

    }

    void USceneCaptureComponent2D::OnRegister()
    {
        ENQUEUE_RENDER_COMMAND(USceneCaptureComponent2D_OnRegister)(
            [this](RenderGraph&) 
            {
                ViewUniformBuffer = RenderGraph::CreatePooledUniformBuffer<FViewShaderParameters>("", nullptr);
            });
    }

    void USceneCaptureComponent2D::OnUnregister()
    {
        ENQUEUE_RENDER_COMMAND(USceneCaptureComponent2D_OnUnregister)(
            [this](RenderGraph&) 
            {
                ViewUniformBuffer = nullptr;
            });
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
        ViewFamily.bIsSceneCapture = true;
        ViewFamily.CaptureSource = CaptureSource;
        if (CaptureSource == SCS_GammaColor)
        {
            ViewFamily.GammaCorrection = 2.2;
            ViewFamily.bEnableToneMapping = true;
        }

        std::array<dvec3, 6> ForwardVectors, UpVectors;
        // The forward and up vectors of cube maps for vulkan and opengl are different
        if (FDynamicRHI::StaticGetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
        {
            ForwardVectors = {
                dvec3(1, 0, 0), 
                dvec3(-1, 0, 0), 
                dvec3(0, 1, 0), 
                dvec3(0, -1, 0), 
                dvec3(0, 0, 1), 
                dvec3(0, 0, -1), 
            };
            UpVectors = {
                dvec3(0, 1, 0), 
                dvec3(0, 1, 0), 
                dvec3(0, 0, -1), 
                dvec3(0, 0, 1), 
                dvec3(0, 1, 0), 
                dvec3(0, 1, 0), 
            };
        }
        else 
        {
            ForwardVectors = {
                dvec3(1, 0, 0), 
                dvec3(-1, 0, 0), 
                dvec3(0, 0, 1), 
                dvec3(0, 0, -1), 
                dvec3(0, 1, 0), 
                dvec3(0, -1, 0), 
            };
            UpVectors = {
                dvec3(0, 0, -1), 
                dvec3(0, 0, -1), 
                dvec3(0, 1, 0), 
                dvec3(0, -1, 0), 
                dvec3(0, 0, -1), 
                dvec3(0, 0, -1), 
            };
        }

        std::vector<FSceneView>& SceneViews = ViewFamily.Views;
        for (int i = 0; i < ViewUniformBuffers.size(); i++)
        {
            FSceneView View(
                ECameraProjectionMode::Perspective,
                glm::radians(90.0), 0,
                0.1, 30000, 
                Position, 
                ForwardVectors[i], 
                UpVectors[i],
                ivec2(Viewport.Width, Viewport.Height));
            SceneViews.push_back(View);
        }  

        FSceneRenderer* SceneRenderer = FSceneRenderer::CreateSceneRenderer(ViewFamily);

        ENQUEUE_RENDER_COMMAND(USceneCaptureComponentCube_UpdateSceneCaptureContents)(
            [SceneRenderer](RenderGraph&) 
            {
                // for (FViewInfo& View : SceneRenderer->Views)
                // {
                //     View.ViewUniformBuffer->UpdateUniformBuffer();
                // }

                RenderGraph& Graph = FRenderingThread::GetRenderGraph();
                SceneRenderer->Render(Graph);
                
                delete SceneRenderer;
            });

    }

    void USceneCaptureComponentCube::OnRegister()
    {
        ENQUEUE_RENDER_COMMAND(USceneCaptureComponentCube_OnRegister)(
            [this](RenderGraph&) 
            {
                for (int i = 0; i < ViewUniformBuffers.size(); i++)
                {
                    ViewUniformBuffers[i] = RenderGraph::CreatePooledUniformBuffer<FViewShaderParameters>("", nullptr);
                }
            });
    }

    void USceneCaptureComponentCube::OnUnregister()
    {
        ENQUEUE_RENDER_COMMAND(USceneCaptureComponentCube_OnUnregister)(
            [this](RenderGraph&) 
            {
                for (int i = 0; i < ViewUniformBuffers.size(); i++)
                {
                    ViewUniformBuffers[i] = nullptr;
                }
            });
    }

}