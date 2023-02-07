#pragma once
#include "PrimitiveComponent.h"

#include "Common/Actor/GeoreferenceActor.h"

namespace nilou {

    UCLASS()
    class UCesium3DTilesetComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:

        UCesium3DTilesetComponent(AActor *InOwner = nullptr);

        virtual void OnRegister() override;

    protected:

        std::string Url;

        AGeoreferenceActor *Georeference;

        double MaximumScreenSpaceError;

        bool bEnableFrustumCulling;

    };




}