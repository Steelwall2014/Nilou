#pragma once
#include "UniformBuffer.h"

namespace nilou {
    BEGIN_UNIFORM_BUFFER_STRUCT(PBRExhibition_UniformBlock)
        SHADER_PARAMETER(float, Red)
        SHADER_PARAMETER(float, Green)
        SHADER_PARAMETER(float, Blue)
        SHADER_PARAMETER(float, Metallic)
        SHADER_PARAMETER(float, Roughness)
    END_UNIFORM_BUFFER_STRUCT()
}