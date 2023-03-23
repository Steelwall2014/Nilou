#include "ViewElementPDI.h"

#include "Scene.h"

namespace nilou {

    void FViewElementPDI::DrawLine(const dvec3 &Start, const dvec3 &End, const vec3 &Color)
    {
        LineElements.emplace_back(Start, End, Color);
    }

    void FViewElementPDI::DrawLine(const FBatchedLine &Line)
    {
        LineElements.push_back(Line);
    }

}