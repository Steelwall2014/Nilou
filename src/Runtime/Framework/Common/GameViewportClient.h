#pragma once
#include <memory>
#include "Viewport.h"

namespace nilou {

    class UGameViewportClient
    {
    public:
        UGameViewportClient();
        std::shared_ptr<class UWorld> World;
        std::shared_ptr<class FScene> Scene;
        void Init();
        void BeginPlay();
        void Tick(double DeltaTime);
        void Draw(FViewport InViewport);
    };

}