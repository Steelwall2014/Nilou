#pragma once
#include "Common/Math/Maths.h"

namespace nilou {
    struct FBatchedLine
    {
        FVector Start;
        FVector End;
        FVector3f Color;
        FBatchedLine()
            : Start(0, 0, 0)
            , End(0, 0, 0)
            , Color(1, 1, 1)
        {

        }

        FBatchedLine(const FVector &InStart, const FVector &InEnd, const FVector3f &InColor = FVector3f(1, 1, 1))
            : Start(InStart)
            , End(InEnd)
            , Color(InColor)
        {

        }
    };
}