#pragma once

#include <memory>
#include <string>
#include <vector>
#include <set>

#include "Common/Scene.h"
#include "Common/Transform.h"
#include "Common/Actor/Actor.h"

namespace nilou {
    using std::vector;

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

        FScene *Scene = nullptr;

        std::set<UCameraComponent*> CameraComponents;
        
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
        Actor->SetActorName(ActorName);
        Actor->PostSpawnInitialize(ActorTransform);
        Actors.push_back(Actor);
        return Actor;
    }

}