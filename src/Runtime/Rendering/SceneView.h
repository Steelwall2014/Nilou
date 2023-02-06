#pragma once

#include <vector>
#include "Frustum.h"

namespace nilou {
    class FSceneView 
    {
    public:
        glm::mat4 ProjectionMatrix;
        glm::mat4 ViewMatrix;
        FViewFrustum ViewFrustum;
        glm::ivec2 ScreenResolution;
    };

    using FSceneLightView = FSceneView;
}