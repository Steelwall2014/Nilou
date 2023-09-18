#include "Actor.h"
#include "Common/Components/ArrowComponent.h"

namespace nilou {

    class NCLASS AArrowActor : public AActor
    {
		GENERATED_BODY()
    public:
        AArrowActor();
		
    protected:

        NPROPERTY()
        std::shared_ptr<UArrowComponent> xArrowComponent;

        NPROPERTY()
        std::shared_ptr<UArrowComponent> yArrowComponent;

        NPROPERTY()
        std::shared_ptr<UArrowComponent> zArrowComponent;
    };

}