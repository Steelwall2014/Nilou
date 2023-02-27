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

    class FOpenGLRenderQuery : public FRHIRenderQuery
    {
    public:

        /** The query resource. */
        GLuint Resource;

        /** The cached query result. */
        GLuint64 Result;

        FOpenGLRenderQuery()
            : Result(0)
        {
            glGenQueries(1, &Resource);
        }

        ~FOpenGLRenderQuery()
        {
            glDeleteQueries(1, &Resource);
        }

    };

    using FOpenGLRenderQueryRef = std::shared_ptr<FOpenGLRenderQuery>;
}