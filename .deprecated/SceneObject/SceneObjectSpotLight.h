#pragma once
#include "SceneObjectLight.h"

namespace und {
    // 聚光灯
    class SceneObjectSpotLight : public SceneObjectLight
    {
    private:
        AttenCurve  m_LightAngleAttenuation;
    public:
        SceneObjectSpotLight() : SceneObjectLight(kSceneObjectTypeSpotLight) 
        {
            // 默认角度衰减，对于角度来说begin_atten和end_atten都是弧度制
            m_LightAngleAttenuation.type = AttenCurveType::kSmooth;
            m_LightAngleAttenuation.u.smooth_params.begin_atten = glm::radians(20.f);// 0.334917597999862;
            m_LightAngleAttenuation.u.smooth_params.end_atten = glm::radians(35.f); // 0.5427974462509155;
        };
        void SetAngleAttenCurve(AttenCurve angle_atten) 
        {
            m_LightAngleAttenuation = angle_atten;
        }
        AttenCurve GetAngleAttenCurve()
        {
            return m_LightAngleAttenuation;
        }

    };
}