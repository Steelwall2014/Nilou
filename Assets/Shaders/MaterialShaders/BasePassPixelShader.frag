#version 460 core
layout (location = 0) out vec4 BaseColor;
layout (location = 1) out vec3 RelativeWorldSpacePosition;
layout (location = 2) out vec3 WorldSpaceNormal;
layout (location = 3) out vec2 MetallicRoughness;
layout (location = 4) out vec3 Emissive;
layout (location = 5) out uint ShadingModel;

layout (std140) uniform FMaterialParameters {
    uint MaterialShadingModel;
};

//#include "../include/Maths.glsl"
//#include "../include/Light.glsl"
//#include "../include/PBRFunctions.glsl"

// To be filled
//#include "../Materials/ColoredMaterial.glsl"


in VS_Out vs_out;
void main()
{
    WorldSpaceNormal = MaterialGetWorldSpaceNormal(vs_out);
//    vec3 TangentSpaceNormal = MaterialGetTangentSpaceNormal(vs_out).rgb;
//    TangentSpaceNormal = normalize(TangentSpaceNormal * 2.0f - 1.0f);   
//    WorldSpaceNormal = normalize(vs_out.TBN * TangentSpaceNormal);
    BaseColor = MaterialGetBaseColor(vs_out);
    RelativeWorldSpacePosition = vs_out.RelativeWorldPosition;
    MetallicRoughness.x = MaterialGetMetallic(vs_out);
    MetallicRoughness.y = MaterialGetRoughness(vs_out);
    Emissive = MaterialGetEmissive(vs_out);
    ShadingModel = MaterialShadingModel;
//    vec3 projCoords = frag_lightspace_pos[0].xyz / frag_lightspace_pos[0].w;
//    projCoords = projCoords * 0.5 + 0.5;
//    float closestDepth = texture(shadowMap, vec3(projCoords.xy, 0)).r; 
//    FragColor = vec4(closestDepth, closestDepth, closestDepth, 1);
}
