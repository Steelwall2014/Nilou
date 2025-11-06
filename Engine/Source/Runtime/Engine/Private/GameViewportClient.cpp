#include "Engine/World.h"
#include "Scene.h"

#include "GameFramework/CameraActor.h"
#include "NObject/Package.h"
#include "GameViewportClient.h"
#include "SceneView.h"
#include "Renderer/RendererModule.h"


namespace nilou {

    UGameViewportClient::UGameViewportClient()
    {
        World = NewObject<UWorld>(GetTransientPackage(), "World");
        Scene = std::make_shared<FScene>();
        World->Scene = Scene.get();
        Scene->World = World;
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
            FSceneView View = LocalPlayer->CalcSceneView(ViewFamily);
            Views.push_back(View);
        }
        GetRendererModule()->BeginRenderingViewFamily(ViewFamily);
    }

}