#version 330 core
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;

out vec3 frag_position;
out vec3 frag_normal;
out vec4 frag_lightspace_pos[5];

uniform mat4 model;
uniform mat4 VP;

struct Light
{
        vec3       lightPosition;
        vec4       lightColor;
        vec4       lightDirection;
        float      lightIntensity;
        bool       lightCastShadow;
        int        lightShadowMapLayerIndex;
        mat4       lightVP;
};
uniform Light lights[5];
uniform int lightCount;
void main()
{
    gl_Position = VP * model * vec4(POSITION, 1.0f);
    frag_position = vec3(model * vec4(POSITION, 1.0f));
    frag_normal = mat3(transpose(inverse(model))) * NORMAL;
    for (int i = 0; i < lightCount; i++)
    {
        if (lights[i].lightCastShadow)
            frag_lightspace_pos[i] = lights[i].lightVP * vec4(frag_position, 1.f);
    }
}