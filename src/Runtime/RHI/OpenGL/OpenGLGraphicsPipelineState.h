#pragma once

#include <glad/glad.h>
#include "OpenGL/OpenGLShader.h"
#include "RHIResources.h"

namespace nilou {
    class OpenGLGraphicsPipelineState : public FRHIGraphicsPipelineState
    {
    public:
        friend class FOpenGLDynamicRHI;
        OpenGLGraphicsPipelineState(const FGraphicsPipelineStateInitializer& InInitializer)
            : FRHIGraphicsPipelineState(InInitializer) { }
    private:
        OpenGLLinkedProgramRef Program;
    };
    using OpenGLGraphicsPipelineStateRef = std::shared_ptr<OpenGLGraphicsPipelineState>;
}