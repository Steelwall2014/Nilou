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

    struct FOpenGLVertexElement
    {
        GLenum Type;
        //GLuint StreamIndex;
        GLuint Offset;
        GLuint Size;
        //GLuint Divisor;
        GLuint Stride;
        uint8 bNormalized;
        uint8 AttributeIndex;
        uint8 bShouldConvertToFloat;
        uint8 Padding;

        FOpenGLVertexElement()
            : Padding(0)
        {
        }
    };

    class FOpenGLVertexDeclaration : public FRHIVertexDeclaration
    {
    public:
        /** Elements of the vertex declaration. */
        std::vector<FOpenGLVertexElement> VertexElements;

        FOpenGLVertexDeclaration()
        {
        }

        /** Initialization constructor. */
        FOpenGLVertexDeclaration(const std::vector<FOpenGLVertexElement>& InElements)
            : VertexElements(InElements)
        {
        }
        
    };
    using FOpenGLVertexDeclarationRef = std::shared_ptr<FOpenGLVertexDeclaration>;
}