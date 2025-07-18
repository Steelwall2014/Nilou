#pragma once

#include <memory>
#include <string>
#include <vector>
#include <set>

#include "Scene.h"
#include "Common/Math/Transform.h"
#include "Common/Actor/Actor.h"

namespace nilou {

    class AActor;
    class UWorld
    {
    public:
        friend AActor;
        UWorld();

        void InitWorld();

        void InitializeActorsForPlay();

        void BeginPlay();

        void Tick(double DeltaTime);

        void Release();

        FScene *Scene = nullptr;

        // Default reflection probe as a fallback
        class AReflectionProbe* SkyboxReflectionProbe = nullptr;

        class ACameraActor* GetFirstCameraActor() const { return CameraActors.empty() ? nullptr : CameraActors[0]; }

        std::vector<ACameraActor*> GetCameraActors() const { return CameraActors; }
        
        /**
        * Find all Actors in the world of the specified class. 
        * This is a slow operation, use with caution e.g. do not use every frame.
        *
        * @tparam ActorClass The specified class.
        *
        * @param OutActors Output vector of Actors of the specified class.
        * @param bIncludeSubclass Subclasses are included if bIncludeSubclass is set to true.
        */
        template<class ActorClass>
        void GetAllActorsOfClass(std::vector<ActorClass *> &OutActors, bool bIncludeSubclasses=true);

        template<class ActorClass>
        std::shared_ptr<ActorClass> SpawnActor(const FTransform &ActorTransform, const std::string &ActorName);

        void SendAllEndOfFrameUpdates();

        bool HasBegunPlay() const { return bHasBegunPlay; }

    private:
        
        std::vector<std::shared_ptr<AActor>> Actors;

        std::vector<ACameraActor*> CameraActors;

        bool bIsWorldInitialized;
        bool bHasBegunPlay;
    };
}

namespace nilou {

    template<class ActorClass>
    void UWorld::GetAllActorsOfClass(std::vector<ActorClass *> &OutActors, bool bIncludeSubclasses)
    {
        for (std::shared_ptr<AActor> Actor : Actors)
        {
            if (bIncludeSubclasses)
            {
                if (Actor->GetClass()->IsChildOf(ActorClass::StaticClass()))
                    OutActors.push_back(static_cast<ActorClass*>(Actor.get()));
            }
            else 
            {
                if (Actor->GetClass() == ActorClass::StaticClass())
                    OutActors.push_back(static_cast<ActorClass*>(Actor.get()));
            }
        }
    }

    template<class ActorClass>
    std::shared_ptr<ActorClass> UWorld::SpawnActor(const FTransform &ActorTransform, const std::string &ActorName)
    {
        static_assert(TIsDerivedFrom<ActorClass, AActor>::Value, "'ActorClass' template parameter to SpawnActor must be derived from AActor");
        std::shared_ptr<ActorClass> Actor = std::make_shared<ActorClass>();
        Actor->SetOwnedWorld(this);
        Actor->Rename(ActorName);
        Actor->PostSpawnInitialize(ActorTransform);
        Actors.push_back(Actor);
        if constexpr (TIsDerivedFrom<ActorClass, ACameraActor>::Value)
        {
            CameraActors.push_back(Actor.get());
        }
        return Actor;
    }

}