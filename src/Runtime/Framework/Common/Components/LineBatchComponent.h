#pragma once
#include "PrimitiveComponent.h"

#include "BatchedLine.h"

namespace nilou {

    UCLASS()
    class ULineBatchComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:
        ULineBatchComponent(AActor *InOwner=nullptr)
            : UPrimitiveComponent(InOwner)
        { 
            SetCastShadow(false);
        }

        void DrawLines(const std::vector<FBatchedLine> &InLines);

        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;

        std::vector<FBatchedLine> BatchedLines;
    };

}