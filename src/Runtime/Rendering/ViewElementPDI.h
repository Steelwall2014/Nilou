#pragma once
#include <vector>

#include "Common/Maths.h"
#include "Common/BatchedLine.h"

namespace nilou {

    class FViewElementPDI
    {
    public:
        friend class FDefferedShadingSceneRenderer;
        void DrawLine(const dvec3& Start,const dvec3& End,const vec3& Color);
        void DrawLine(const FBatchedLine &Line);

    private:
        std::vector<FBatchedLine> LineElements;
    };

}