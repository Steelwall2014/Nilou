#include "DepthRendering.h"

namespace nilou {

IMPLEMENT_SHADER_TYPE(FDepthOnlyPS, "/Shaders/Private/MaterialShaders/DepthOnlyPixelShader.slang", "Main", EShaderFrequency::Pixel);

}