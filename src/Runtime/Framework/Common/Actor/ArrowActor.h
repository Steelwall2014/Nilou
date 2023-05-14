#include "Actor.h"
#include "Common/Components/ArrowComponent.h"

namespace nilou {

    class NCLASS AArrowActor : public AActor
    {
		GENERATE_BODY()
    public:
        AArrowActor();
		
    protected:

        std::shared_ptr<UArrowComponent> xArrowComponent;

        std::shared_ptr<UArrowComponent> yArrowComponent;

        std::shared_ptr<UArrowComponent> zArrowComponent;
    };

}