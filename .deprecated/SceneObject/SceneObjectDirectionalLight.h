#pragma once
#include "SceneObjectLight.h"

namespace und {
    // Æ½ÐÐ¹â
	class SceneObjectDirectionalLight : public SceneObjectLight
    {
    public:
        SceneObjectDirectionalLight()
            : SceneObjectLight(SceneObjectType::kSceneObjectTypeDirectionalLight) 
        {
            m_Intensity = 100.f;
            m_LightColor = glm::vec4(222.0, 188.0, 123.0, 255.0) / glm::vec4(255.0);// 244 / 255.f, 253 / 255.f, 253 / 255.f, 1.f);
            m_LightDistAttenuation.type = AttenCurveType::kNone;
            memset(&m_LightDistAttenuation.u, 0, sizeof(m_LightDistAttenuation.u));
        }
    };
}