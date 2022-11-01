struct Light
{
        float      lightDistAttenCurveParams[5];
        float      lightAngleAttenCurveParams[5];
        mat4       lightVP;
        vec4       lightColor;
        vec3       lightPosition;
        vec3       lightDirection;
        int        lightType;
        float      lightIntensity;
        int        lightDistAttenCurveType;
        int        lightAngleAttenCurveType;
        bool       lightCastShadow;
        int        lightShadowMapLayerIndex;
};

float LinearAtten(float t, float begin_atten, float end_atten)
{
    if (t < begin_atten)
    {
        return 1.0f;
    }
    else if (t > end_atten)
    {
        return 0.0f;
    }
    else
    {
        return (end_atten - t) / (end_atten - begin_atten);
    }
}
float SmoothAtten(float t, float begin_atten, float end_atten)
{
    float tmp = LinearAtten(t, begin_atten, end_atten);
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

float apply_atten_curve(float t, int atten_curve_type, float atten_params[5])
{
    float atten = 1.f;
    switch (atten_curve_type)
    {
    case 1:     // Linear
        atten = LinearAtten(t, atten_params[0], atten_params[1]);
        break;
    case 2:     // Smooth
        atten = SmoothAtten(t, atten_params[0], atten_params[1]);
        break;
    case 3:     // Inverse
        atten = InverseAtten(t, atten_params[0], atten_params[1], atten_params[2], atten_params[3]);
        break;
    case 4:     // InverseSquare
        atten = InverseSquareAtten(t, atten_params[0], atten_params[1], atten_params[2], atten_params[3], atten_params[4]);
        break;
    case 0:     // None
        break;
    default:
        break;
    }
    return atten;
}