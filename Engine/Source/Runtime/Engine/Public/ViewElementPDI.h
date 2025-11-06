#pragma once
#include <vector>

#include "Math/Maths.h"
#include "BatchedLine.h"

namespace nilou {

    class FViewElementPDI
    {
    public:
        friend class FDeferredShadingSceneRenderer;
        void DrawLine(const FVector& Start,const FVector& End,const FVector3f& Color);
        void DrawLine(const FBatchedLine &Line);

    private:
        std::vector<FBatchedLine> LineElements;
    };

}