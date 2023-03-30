#include "SceneCaptureComponent.h"
#include "Common/Actor/Actor.h"

namespace nilou {

    void USceneCaptureComponent::HideComponent(std::weak_ptr<UPrimitiveComponent> InComponent)
    {
        if (!InComponent.expired())
        {
            HiddenComponents.push_back(InComponent);
        }
    }

    void USceneCaptureComponent::HideActorComponents(std::weak_ptr<AActor> InActor)
    {
        if (!InActor.expired())
        {
            auto Actor = InActor.lock();
            std::vector<std::weak_ptr<UPrimitiveComponent>> PrimitiveComponents;
            Actor->GetComponents(PrimitiveComponents);
            for (auto WeakPrimComp : PrimitiveComponents)
            {
                HiddenComponents.push_back(WeakPrimComp);
            }
        }
    }

}