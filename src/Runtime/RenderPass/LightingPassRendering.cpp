#include "LightingPassRendering.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FLightingPassVS, "/Shaders/GlobalShaders/LightingPassVertexShader.vert", EShaderFrequency::SF_Vertex, Global);
    IMPLEMENT_SHADER_TYPE(FLightingPassPS, "/Shaders/GlobalShaders/LightingPassPixelShader.frag", EShaderFrequency::SF_Pixel, Global);
}