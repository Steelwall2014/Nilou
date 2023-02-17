#include "ViewElementPDI.h"

#include "Common/Scene.h"

namespace nilou {

    void FViewElementPDI::DrawLine(const dvec3 &Start, const dvec3 &End, const vec3 &Color)
    {
        ViewInfo->LineElements.push_back(FBatchedLine{Start, End, Color});
    }

}