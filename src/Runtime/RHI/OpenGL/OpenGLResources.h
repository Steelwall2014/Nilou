#pragma once
#include "OpenGL/OpenGLBuffer.h"
#include "OpenGL/OpenGLFramebuffer.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/OpenGLTexture.h"
// #include "OpenGL/OpenGLVertexArrayObject.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "glad/glad.h"
#include <memory>

namespace nilou {

    class OpenGLVertexDeclaration : public FRHIVertexDeclaration
    {
    public:
        struct Element
        {
            uint8 StreamIndex;
            uint8 Offset;
            uint8 AttributeIndex;
            uint16 Stride;
            GLenum DataType;
            GLint Size;
            bool bNormalized;
            bool bShouldConvertToFloat;
        };
        std::vector<Element> Elements;
    };
    using OpenGLVertexDeclarationRef = std::shared_ptr<OpenGLVertexDeclaration>;
}