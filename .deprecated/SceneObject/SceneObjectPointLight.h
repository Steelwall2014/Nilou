#pragma once
#include "SceneObjectLight.h"

namespace und {
    // ·º¹âµÆ
    class SceneObjectPointLight : public SceneObjectLight
    {
    public:
        SceneObjectPointLight()
            : SceneObjectLight(SceneObjectType::kSceneObjectTypePointLight) {}
    };
}