#version 460

#include "../include/ViewShaderParameters.glsl"

void main()
{
    VS_Out vs_out;
    FVertexFactoryIntermediates VFIntermediates = VertexFactoryIntermediates();
    vs_out.TexCoords = VertexFactoryGetTexCoord(VFIntermediates);
    dvec3 WorldPosition = VertexFactoryGetWorldPosition(VFIntermediates);
    vs_out.WorldPosition = vec3(WorldPosition - CameraPosition);
    vs_out.WorldPosition += MaterialGetWorldSpaceOffset(vs_out);
    vs_out.ClipPosition = RelWorldToClip * vec4(vs_out.WorldPosition, 1);
    gl_Position = vs_out.ClipPosition;
}
