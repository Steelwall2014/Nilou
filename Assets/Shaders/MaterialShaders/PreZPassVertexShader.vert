#version 460

#include "../include/ViewShaderParameters.glsl"

void main()
{
    VS_Out vs_out;
    FVertexFactoryIntermediates VFIntermediates = VertexFactoryIntermediates();
    vs_out.TexCoords = VertexFactoryGetTexCoord(VFIntermediates);
    dvec3 WorldPosition = VertexFactoryGetWorldPosition(VFIntermediates);
    vs_out.RelativeWorldPosition = vec3(WorldPosition - CameraPosition);
    vs_out.RelativeWorldPosition += MaterialGetWorldSpaceOffset(vs_out);
    vs_out.ClipPosition = RelWorldToClip * vec4(vs_out.RelativeWorldPosition, 1);
    gl_Position = vs_out.ClipPosition;
}
