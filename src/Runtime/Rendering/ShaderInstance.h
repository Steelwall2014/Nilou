#pragma once

#include <memory>
#include <vector>
#include <glslang/Public/ShaderLang.h>
#include "RHIResources.h"
#include "ShaderParameter.h"
#include "Shader.h"
#include "RenderResource.h"

namespace nilou {
    class FShaderInstance : public FRenderResource
    {
    public:

        FShaderInstance(
            const std::string& InShaderName, 
            const std::string& InCode, 
            EPipelineStage InPipelineStage,
            EShaderMetaType InShaderMetaType)
            : ShaderName(InShaderName)
            , Code(InCode)
            , PipelineStage(InPipelineStage)
            , ShaderMetaType(InShaderMetaType)
        { }

        std::unique_ptr<glslang::TShader> ShaderGlsl;
        RHIShaderRef ShaderRHI;
        EPipelineStage PipelineStage;
        EShaderMetaType ShaderMetaType; // Material or Global
        std::string ShaderName;
        std::string Code;

        virtual void InitRHI() override;

        virtual void ReleaseRHI() override;

    };
    using FShaderInstanceRef = std::shared_ptr<FShaderInstance>;
}