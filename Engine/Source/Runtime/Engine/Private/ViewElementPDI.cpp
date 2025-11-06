#include "ViewElementPDI.h"

namespace nilou {

    void FViewElementPDI::DrawLine(const FVector &Start, const FVector &End, const FVector3f &Color)
    {
        LineElements.emplace_back(Start, End, Color);
    }

    void FViewElementPDI::DrawLine(const FBatchedLine &Line)
    {
        LineElements.push_back(Line);
    }

}