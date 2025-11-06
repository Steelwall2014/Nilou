#include "Scene.h"
#include "Engine/World.h"
#include "Renderer/RendererModule.h"
#include "SceneView.h"
#include "Renderer/DeferredShadingSceneRenderer.h"

namespace nilou {

    void FRendererModule::BeginRenderingViewFamily(const FSceneViewFamily& ViewFamily)
    {
        FScene* Scene = ViewFamily.Scene;
        if (Scene)
        {
            UWorld *World = Scene->World;
            if (World)
                World->SendAllEndOfFrameUpdates();
            Scene->IncrementFrameNumber();
		    const_cast<FSceneViewFamily&>(ViewFamily).FrameNumber = Scene->GetFrameNumber();
        }

        FSceneRenderer* SceneRenderer = FSceneRenderer::CreateSceneRenderer(ViewFamily);

        USceneCaptureComponent::UpdateDeferredCaptures(Scene);

        ENQUEUE_RENDER_COMMAND(FRendererModule_BeginRenderingViewFamily)(
            [Scene, SceneRenderer, ViewFamily](RenderGraph& Graph)
            {
                Scene->UpdateRenderInfos(Graph);
                SceneRenderer->Render(Graph);
                delete SceneRenderer;
            });
    }

    FRendererModule* GetRendererModule()
    {
        static FRendererModule* RendererModule = new FRendererModule;
        return RendererModule;
    }

}