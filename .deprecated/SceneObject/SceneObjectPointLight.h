#pragma once
#include "SceneObjectLight.h"

namespace und {
    // �����
    class SceneObjectPointLight : public SceneObjectLight
    {
    public:
        SceneObjectPointLight()
            : SceneObjectLight(SceneObjectType::kSceneObjectTypePointLight) {}
    };
}