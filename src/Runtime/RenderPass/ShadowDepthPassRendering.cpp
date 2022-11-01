#include "ShadowDepthPassRendering.h"
#include "Shader.h"
#include "Templates/ObjectMacros.h"

namespace nilou {

    IMPLEMENT_SHADER_TYPE(FShadowDepthVS, "/Shaders/MaterialShaders/ShadowDepthVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FShadowDepthPS, "/Shaders/MaterialShaders/ShadowDepthPixelShader.frag", EShaderFrequency::SF_Pixel, Material);

}