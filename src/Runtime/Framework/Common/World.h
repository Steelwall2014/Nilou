#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Common/Scene.h"
#include "Common/Transform.h"

namespace nilou {
    using std::vector;

    class AActor;
    class UWorld
    {
    public:
        UWorld();

        void BeginPlay();

        void Tick(double DeltaTime);

        FScene *Scene = nullptr;
        
        std::vector<std::shared_ptr<AActor>> Actors;

        template<class ActorClass>
        std::shared_ptr<ActorClass> SpawnActor(const FTransform &ActorTransform, const std::string &ActorName);

        void SendAllEndOfFrameUpdates();
    };
}