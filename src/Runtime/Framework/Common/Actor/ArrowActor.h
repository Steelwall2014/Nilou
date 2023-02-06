#include "Actor.h"
#include "Common/Components/ArrowComponent.h"

namespace nilou {

	UCLASS()
    class AArrowActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        AArrowActor();
		
    protected:

        std::shared_ptr<UArrowComponent> xArrowComponent;

        std::shared_ptr<UArrowComponent> yArrowComponent;

        std::shared_ptr<UArrowComponent> zArrowComponent;
    };

}