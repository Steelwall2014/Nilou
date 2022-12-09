#include "SkyAtmospherePassRendering.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FSkyAtmosphereVS, "", EShaderFrequency::SF_Vertex, Global)
    IMPLEMENT_SHADER_TYPE(FSkyAtmospherePS, "", EShaderFrequency::SF_Pixel, Global)
}