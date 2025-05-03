#pragma once
#include "Common/Math/Maths.h"

namespace nilou {
    struct FBatchedLine
    {
        dvec3 Start;
        dvec3 End;
        vec3 Color;
        FBatchedLine()
            : Start(0, 0, 0)
            , End(0, 0, 0)
            , Color(1, 1, 1)
        {

        }

        FBatchedLine(const dvec3 &InStart, const dvec3 &InEnd, const vec3 &InColor = vec3(1, 1, 1))
            : Start(InStart)
            , End(InEnd)
            , Color(InColor)
        {

        }
    };
}