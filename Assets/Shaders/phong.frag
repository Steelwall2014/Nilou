#version 330 core

layout (location = 0) out vec4 FragColor;

in vec3 frag_position;
in vec3 frag_normal;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform float Kd;
uniform float Ks;
uniform float Ka;
uniform float shininess;

void main()
{
    vec3 norm = normalize(frag_normal);
    vec3 V = normalize(cameraPos - frag_position);
    vec3 LightDir = normalize(lightPos - frag_position);
    vec3 R = -LightDir - norm * (2.0 * dot(norm, -LightDir));
    float LN = dot(norm, LightDir);
    float RV = dot(R, V);
    vec3 Id = lightColor * Kd * max(0, LN);
    vec3 Is = lightColor * Ks * pow(max(0, RV), shininess);
    vec3 Ia = lightColor * Ka;
    FragColor = vec4((Ia + Id + Is) * objectColor, 1);
}