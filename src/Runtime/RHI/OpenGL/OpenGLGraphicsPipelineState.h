#pragma once

#include <glad/glad.h>
#include "OpenGL/OpenGLShader.h"
#include "RHIResources.h"

namespace nilou {
    class OpenGLGraphicsPipelineState : public FRHIGraphicsPipelineState
    {
        friend class FOpenGLDynamicRHI;
    private:
        OpenGLLinkedProgramRef Program;
    };
    using OpenGLGraphicsPipelineStateRef = std::shared_ptr<OpenGLGraphicsPipelineState>;
}