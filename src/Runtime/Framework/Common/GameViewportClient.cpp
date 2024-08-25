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
        ViewFamily.GammaCorrection = 2.2;
        ViewFamily.bEnableToneMapping = true;
        std::vector<FSceneView> &Views = ViewFamily.Views;
        std::vector<ACameraActor*> CameraActors;
        World->GetAllActorsOfClass(CameraActors);

        for (ACameraActor* LocalPlayer : CameraActors)
        {
            FSceneView View = LocalPlayer->CalcSceneView(&ViewFamily);
            Views.push_back(View);
        }
        GetRendererModule()->BeginRenderingViewFamily(ViewFamily);
    }

}