#include "SceneObjectLight.h"


und::SceneObjectLight::SceneObjectLight(const SceneObjectType type)
    : BaseSceneObject(type)
{
    m_LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_Intensity = 1000.f;
    m_bCastShadows = true;

    // Ä¬ÈÏ¾àÀëË¥¼õ
    m_LightDistAttenuation.type = AttenCurveType::kInverseSquare;
    m_LightDistAttenuation.u.inverse_squre_params.scale = 1.f;
    m_LightDistAttenuation.u.inverse_squre_params.offset = 0.0f;
    m_LightDistAttenuation.u.inverse_squre_params.kq = 1.f;
    m_LightDistAttenuation.u.inverse_squre_params.kl = 0.f;
    m_LightDistAttenuation.u.inverse_squre_params.kc = 0.f;

    m_fNearClipDistance = 1.f;
    m_fFarClipDistance = 100.f;

    m_ShadowMapResolution.x = m_ShadowMapResolution.y = 1024;
}

glm::vec4 und::SceneObjectLight::GetLightColor()
{
    return m_LightColor;
}

float und::SceneObjectLight::GetIntensity()
{
    return m_Intensity;
}

bool und::SceneObjectLight::GetCastShadows()
{
    return m_bCastShadows;
}

float und::SceneObjectLight::GetNearClip()
{
    return m_fNearClipDistance;
}

float und::SceneObjectLight::GetFarClip()
{
    return m_fFarClipDistance;
}

void und::SceneObjectLight::SetDistAttenCurve(AttenCurve dist_atten_angle)
{
    m_LightDistAttenuation = dist_atten_angle;
}

void und::SceneObjectLight::SetLightColor(glm::vec4 light_color)
{
    m_LightColor = light_color;
}

und::AttenCurve und::SceneObjectLight::GetDistAttenCurve()
{
    return m_LightDistAttenuation;
}

glm::ivec2 und::SceneObjectLight::GetShadowMapResolution()
{
    return m_ShadowMapResolution;
}
