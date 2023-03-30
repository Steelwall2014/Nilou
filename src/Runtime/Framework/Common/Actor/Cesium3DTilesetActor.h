#pragma once
#include "Actor.h"
#include "GeoreferenceActor.h"

#include "Common/Components/Cesium3DTilesetComponent.h"

namespace nilou {

	UCLASS()
    class ACesiumTilesetActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        ACesiumTilesetActor()
        {
            TilesetComponent = CreateComponent<UCesium3DTilesetComponent>(this);
            TilesetComponent->AttachToComponent(GetRootComponent());
        }

        UCesium3DTilesetComponent *GetTilesetComponent() const { return TilesetComponent.get(); }

    private:

        std::shared_ptr<UCesium3DTilesetComponent> TilesetComponent;

    };

}