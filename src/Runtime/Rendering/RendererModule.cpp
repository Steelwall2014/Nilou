#include "Scene.h"
#include "Common/World.h"
#include "RendererModule.h"
#include "SceneView.h"
#include "DeferredShadingSceneRenderer.h"

namespace nilou {

    void FRendererModule::BeginRenderingViewFamily(FSceneViewFamily* ViewFamily)
    {
        FScene* Scene = ViewFamily->Scene;
        if (Scene)
        {
            UWorld *World = Scene->World;
            if (World)
                World->SendAllEndOfFrameUpdates();
            Scene->IncrementFrameNumber();
		    ViewFamily->FrameNumber = Scene->GetFrameNumber();
        }

        FSceneRenderer* SceneRenderer = FSceneRenderer::CreateSceneRenderer(*ViewFamily);

        USceneCaptureComponent::UpdateDeferredCaptures(Scene);

        ENQUEUE_RENDER_COMMAND(FRendererModule_BeginRenderingViewFamily)(
            [Scene, SceneRenderer, ViewFamily](RenderGraph& Graph)
            {
                Scene->UpdateRenderInfos(Graph);
                SceneRenderer->Render(Graph);
                delete SceneRenderer;
                delete ViewFamily;
            });
    }

    FRendererModule* GetRendererModule()
    {
        static FRendererModule* RendererModule = new FRendererModule;
        return RendererModule;
    }

}