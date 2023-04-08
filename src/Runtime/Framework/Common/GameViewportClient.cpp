#include "World.h"
#include "Scene.h"

#include "Common/Actor/CameraActor.h"
#include "GameViewportClient.h"
#include "SceneView.h"
#include "RendererModule.h"


namespace nilou {

    UGameViewportClient::UGameViewportClient()
    {
        World = std::make_shared<UWorld>();
        Scene = std::make_shared<FScene>();
        World->Scene = Scene.get();
        Scene->World = World.get();
    }

    void UGameViewportClient::Init()
    {
        World->InitWorld();
    }

    void UGameViewportClient::BeginPlay()
    {
        World->BeginPlay();
    }

    void UGameViewportClient::Tick(double DeltaTime)
    {
        World->Tick(DeltaTime);
    }

    void UGameViewportClient::Draw(FViewport InViewport)
    {
        FSceneViewFamily ViewFamily(InViewport, Scene.get());
        std::vector<FSceneView*> &Views = ViewFamily.Views;
        std::vector<ACameraActor*> CameraActors;
        World->GetAllActorsOfClass(CameraActors);

        for (ACameraActor* LocalPlayer : CameraActors)
        {
            FSceneView* View = LocalPlayer->CalcSceneView(&ViewFamily);
            if (View)
            {
                Views.push_back(View);
                // We only take the first view
                break;
            }
        }
        GetRendererModule()->BeginRenderingViewFamily(&ViewFamily);
        for (FSceneView* View : ViewFamily.Views)
        {
            delete View;
        }
    }

}