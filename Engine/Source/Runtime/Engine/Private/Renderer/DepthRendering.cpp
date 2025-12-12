#include "DepthRendering.h"

namespace nilou {

IMPLEMENT_SHADER_TYPE(FDepthOnlyPS, "/Shaders/MaterialShaders/DepthOnlyPixelShader.slang", "Main", EShaderFrequency::Pixel);

}