#pragma once

#include <vector>
#include "Frustum.h"

namespace nilou {
    class FSceneView 
    {
    public:
        FViewFrustum ViewFrustum;
        glm::dmat4 ProjectionMatrix;
        glm::dmat4 ViewMatrix;
        dvec3 Position;
        dvec3 Forward;
        dvec3 Up;
        double AspectRatio;
        double VerticalFieldOfView;
        double NearClipDistance;
        double FarClipDistance;
        glm::ivec2 ScreenResolution;
    };

    using FSceneLightView = FSceneView;
}