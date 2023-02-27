#ifndef LIGHTSHADERPARAMETERS_H
#define LIGHTSHADERPARAMETERS_H


#define LT_None (0)
#define LT_Spot (1)
#define LT_Directional (2)
#define LT_Point (3)



struct FLightAttenParameters
{
    vec4 AttenCurveParams;
    float AttenCurveScale;
    int AttenCurveType;
};


struct FLightShaderParameters
{
    FLightAttenParameters lightDistAttenParams;
    FLightAttenParameters lightAngleAttenParams;
    dvec3   lightPosition;
    vec4    lightColor;
    vec3    lightDirection;
    int     lightType;
    float   lightIntensity;
    bool    lightCastShadow;
};

float LinearAtten(float t, float scale, float begin_atten, float end_atten)
{
    if (t < begin_atten)
    {
        return 1.0f * scale;
    }
    else if (t > end_atten)
    {
        return 0.0f;
    }
    else
    {
        return (end_atten - t) / (end_atten - begin_atten) * scale;
    }
}
float SmoothAtten(float t, float scale, float begin_atten, float end_atten)
{
    float tmp = LinearAtten(t, scale, begin_atten, end_atten);
    float atten = 3.0f * pow(tmp, 2.0f) - 2.0f * pow(tmp, 3.0f);
    return atten;
}
float InverseAtten(float t, float scale, float offset, float kl, float kc)
{
    float atten = clamp((scale / ((kl * t) + (kc * scale))) + offset, 0.0f, 1.0f);
    return atten;
}
float InverseSquareAtten(float t, float scale, float offset, float kq, float kl, float kc)
{
    float atten = clamp(pow(scale, 2.0f) / (kq * pow(t, 2.0f) + kl*t*scale + kc*pow(scale, 2.0f)) + offset, 
        0.0f, 1.0f);
    return atten;
}

float apply_atten_curve(float t, FLightAttenParameters lightAttenParams)
{
    float atten = 1.f;
    vec4 params = lightAttenParams.AttenCurveParams;
    float scale = lightAttenParams.AttenCurveScale;
    switch (lightAttenParams.AttenCurveType)
    {
    case 1:     // Linear
        atten = LinearAtten(t, scale, params.x, params.y);
        break;
    case 2:     // Smooth
        atten = SmoothAtten(t, scale, params.x, params.y);
        break;
    case 3:     // Inverse
        atten = InverseAtten(t, scale, params.x, params.y, params.z);
        break;
    case 4:     // InverseSquare
        atten = InverseSquareAtten(t, scale, params.x, params.y, params.z, params.w);
        break;
    case 0:     // None
        break;
    default:
        break;
    }
    return atten;
}
#endif