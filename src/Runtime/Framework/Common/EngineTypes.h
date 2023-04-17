#pragma once

namespace nilou {

    enum ESceneCaptureSource
    {
        SCS_LinearColor,
        SCS_GammaColor,
        SCS_SceneDepth // SceneDepth in R    
    };

    enum class ECameraProjectionMode
    {
        Perspective,
        Orthographic
    };
    
}