#pragma once

#include <memory>
#include <vector>
#include <glslang/Public/ShaderLang.h>
#include "RHIResources.h"
#include "ShaderParameter.h"
#include "Shader.h"

namespace nilou {
    class FShaderInstance
    {
    public:
        std::unique_ptr<glslang::TShader> ShaderGlsl;
        RHIShaderRef ShaderRHI;
        EPipelineStage PipelineStage;
        EShaderMetaType ShaderMetaType; // Material or Global
        std::string ShaderName;

    };
    using FShaderInstanceRef = std::shared_ptr<FShaderInstance>;
}