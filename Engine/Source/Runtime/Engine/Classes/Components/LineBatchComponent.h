#pragma once
#include "PrimitiveComponent.h"

#include "BatchedLine.h"

namespace nilou {

    class NCLASS ULineBatchComponent : public UPrimitiveComponent
    {
        GENERATED_BODY()
    public:
        ULineBatchComponent()
        { 
            SetCastShadow(false);
        }

        void DrawLines(const std::vector<FBatchedLine> &InLines);

        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;

        std::vector<FBatchedLine> BatchedLines;
    };

}