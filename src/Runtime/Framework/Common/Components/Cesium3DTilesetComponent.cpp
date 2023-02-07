#include "Common/World.h"
#include "Cesium3DTilesetComponent.h"

namespace nilou {

    UCesium3DTilesetComponent::UCesium3DTilesetComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
        , Url("")
        , Georeference(nullptr)
        , MaximumScreenSpaceError(16.0)
        , bEnableFrustumCulling(true)
    {
    }

    void UCesium3DTilesetComponent::OnRegister()
    {
        if (Georeference == nullptr && WorldPrivate != nullptr)
        {
            std::vector<AGeoreferenceActor*> Georeferences;
            WorldPrivate->GetAllActorsOfClass<AGeoreferenceActor>(Georeferences, false);
            if (!Georeferences.empty())
            {
                this->Georeference = Georeferences[0];
            }
            else 
            {
                this->Georeference = WorldPrivate->SpawnActor<AGeoreferenceActor>(FTransform::Identity, "CesiumGeoreference").get();
            }
        }

        UPrimitiveComponent::OnRegister();
    }

}