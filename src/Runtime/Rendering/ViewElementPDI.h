#pragma once
#include <vector>

#include "Common/Math/Maths.h"
#include "BatchedLine.h"

namespace nilou {

    class FViewElementPDI
    {
    public:
        friend class FDeferredShadingSceneRenderer;
        void DrawLine(const dvec3& Start,const dvec3& End,const vec3& Color);
        void DrawLine(const FBatchedLine &Line);

    private:
        std::vector<FBatchedLine> LineElements;
    };

}