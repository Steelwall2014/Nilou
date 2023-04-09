#include "Scene.h"
#include "Common/World.h"
#include "RendererModule.h"
#include "SceneView.h"
#include "DefferedShadingSceneRenderer.h"

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

        FSceneRenderer* SceneRenderer = FSceneRenderer::CreateSceneRenderer(ViewFamily);

        USceneCaptureComponent::UpdateDeferredCaptures(Scene);

        ENQUEUE_RENDER_COMMAND(FRendererModule_BeginRenderingViewFamily)(
            [Scene, SceneRenderer](FDynamicRHI* RHICmdList)
            {
                Scene->UpdateRenderInfos();
                SceneRenderer->Render();
                delete SceneRenderer;
            });
    }

    FRendererModule* GetRendererModule()
    {
        static FRendererModule* RendererModule = new FRendererModule;
        return RendererModule;
    }

}