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
        std::set<FShaderParameterInfo> Parameters;
        RHIShaderRef ShaderRHI;
        EPipelineStage PipelineStage;
        EShaderMetaType ShaderMetaType; // Material or Global

        RHIComputeShader *GetComputeShader() 
        { 
            if (PipelineStage == EPipelineStage::PS_Compute) 
                return static_cast<RHIComputeShader*>(ShaderRHI.get());
            else
                return nullptr; 
        }
#ifdef NILOU_DEBUG
        std::string DebugCode;
#endif
    };
    using FShaderInstanceRef = std::shared_ptr<FShaderInstance>;
}