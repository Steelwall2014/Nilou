#include "ArrowActor.h"

namespace nilou {

    AArrowActor::AArrowActor()
    {
        xArrowComponent = CreateComponent<UArrowComponent>(this, "XArrowComponent");
        xArrowComponent->SetArrowColor(vec4(1, 0, 0, 1));
        xArrowComponent->SetRelativeRotation(FRotator(0, 0, 0));
        yArrowComponent = CreateComponent<UArrowComponent>(this, "YArrowComponent");
        yArrowComponent->SetArrowColor(vec4(0, 1, 0, 1));
        yArrowComponent->SetRelativeRotation(FRotator(0, 90, 0));
        zArrowComponent = CreateComponent<UArrowComponent>(this, "ZArrowComponent");
        zArrowComponent->SetArrowColor(vec4(0, 0, 1, 1));
        zArrowComponent->SetRelativeRotation(FRotator(90, 0, 0));

        
        xArrowComponent->AttachToComponent(GetRootComponent());
        yArrowComponent->AttachToComponent(GetRootComponent());
        zArrowComponent->AttachToComponent(GetRootComponent());
    }

}