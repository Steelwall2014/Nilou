#ifndef VIEWSHADERPARAMETERS_H
#define VIEWSHADERPARAMETERS_H
layout (std140, binding=48) uniform FViewShaderParameters {
    dvec4 FrustumPlanes[6];
    mat4 RelWorldToView;
    mat4 ViewToClip;
    mat4 RelWorldToClip;
    mat4 ClipToView;
    mat4 RelClipToWorld;
    mat4 AbsWorldToClip;
    dvec3 CameraPosition;
    vec3 CameraDirection;
    ivec2 CameraResolution;
    float CameraNearClipDist;
    float CameraFarClipDist;
    float CameraVerticalFieldOfView;
};
#endif