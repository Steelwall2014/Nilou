#pragma once
#include "PrimitiveComponent.h"

#include "Common/BatchedLine.h"

namespace nilou {

    UCLASS()
    class ULineBatchComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:
        ULineBatchComponent(AActor *InOwner)
            : UPrimitiveComponent(InOwner)
        { 
        }

        void DrawLines(const std::vector<FBatchedLine> &InLines);

        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;

        std::vector<FBatchedLine> BatchedLines;
    };

}