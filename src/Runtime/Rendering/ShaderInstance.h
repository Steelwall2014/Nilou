#pragma once

#include <memory>
#include <vector>
#include "RHIResources.h"
#include "ShaderParameter.h"
#include "Shader.h"

namespace nilou {
    class FShaderInstance
    {
    public:
        std::map<std::string, FShaderParameterInfo> ParameterMap;
        RHIShaderRef Shader;
        EPipelineStage PipelineStage;
        EShaderMetaType ShaderMetaType; // Material or Global

#ifdef _DEBUG
        std::string DebugCode;
#endif
    };
    using FShaderInstanceRef = std::shared_ptr<FShaderInstance>;
}