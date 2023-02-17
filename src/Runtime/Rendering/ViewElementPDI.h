#pragma once
#include "Common/Maths.h"

namespace nilou {

    class FViewElementPDI
    {
    public:
        void DrawLine(const dvec3& Start,const dvec3& End,const vec3& Color);

    private:
        class FViewSceneInfo *ViewInfo;
    };

}