#pragma once
#include "Shader.h"
#include "Templates/ObjectMacros.h"

namespace nilou {

    class FShadowDepthVS : public FMaterialShader
    {
        DECLARE_SHADER_TYPE()
    };

    class FShadowDepthPS : public FMaterialShader
    {
        DECLARE_SHADER_TYPE()
    };
}