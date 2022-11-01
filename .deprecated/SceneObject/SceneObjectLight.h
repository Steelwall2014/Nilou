#pragma once
#include "Common/BaseSceneObject.h"

namespace und {
    enum AttenCurveType {
        kNone = 0, 
        kLinear = 1,
        kSmooth = 2,
        kInverse = 3,
        kInverseSquare = 4
    };
    struct AttenCurve {
        AttenCurveType type{ AttenCurveType::kNone };
        union AttenCurveParams {
            struct LinearParam {
                float begin_atten;
                float end_atten;
            } linear_params;
            struct SmoothParam {
                float begin_atten;
                float end_atten;
            } smooth_params;
            struct InverseParam {
                float scale;
                float offset;
                float kl;
                float kc;
            } inverse_params;
            struct InverseSquareParam {
                float scale;
                float offset;
                float kq;
                float kl;
                float kc;
            } inverse_squre_params;
        } u;

        AttenCurve() = default;
    };
    // π‚‘¥
    class SceneObjectLight : public BaseSceneObject
    {
    protected:
        glm::vec4   m_LightColor;
        float       m_Intensity;
        AttenCurve  m_LightDistAttenuation;
        float       m_fNearClipDistance;
        float       m_fFarClipDistance;
        bool        m_bCastShadows;
        glm::ivec2  m_ShadowMapResolution;
        SceneObjectLight(const SceneObjectType type);

    public:
        glm::vec4   GetLightColor();
        float       GetIntensity();
        bool        GetCastShadows();
        float       GetNearClip();
        float       GetFarClip();
        void        SetDistAttenCurve(AttenCurve dist_atten_angle);
        void        SetLightColor(glm::vec4 light_color);
        AttenCurve  GetDistAttenCurve();
        glm::ivec2  GetShadowMapResolution();
    };
}