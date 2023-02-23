#pragma once
#include <memory>
#include <glad/glad.h>

#include "Common/EnumClassFlags.h"
#include "OpenGL/OpenGLBuffer.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"

namespace nilou {
    class OpenGLBuffer : public RHIBuffer
    {
    public:
		GLuint	Resource;
		GLenum	Target;

        OpenGLBuffer(
            uint32 InStride,
            uint32 InSize,
            EBufferUsageFlags InUsage,
            void *InData
        )
        : RHIBuffer(InStride, InSize, InUsage)
        , Resource(0)
        , Target(0)
        {
            Target = GL_ARRAY_BUFFER;
            if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::IndexBuffer))
                Target = GL_ELEMENT_ARRAY_BUFFER;
            else if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::StructuredBuffer))
                Target = GL_SHADER_STORAGE_BUFFER;
            else if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::DrawIndirect))
                Target = GL_DRAW_INDIRECT_BUFFER;
            else if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::DispatchIndirect))
                Target = GL_DISPATCH_INDIRECT_BUFFER;
            else if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::AtomicCounter))
                Target = GL_ATOMIC_COUNTER_BUFFER;
            
            GLenum GLUsage = GL_STATIC_DRAW;
            if (EnumHasAnyFlags(InUsage, EBufferUsageFlags::Dynamic))
                GLUsage = GL_DYNAMIC_DRAW;
            glGenBuffers(1, &Resource);
            glBindBuffer(Target, Resource);
            glBufferData(Target, InSize, InData, GLUsage);
            glBindBuffer(Target, 0);
        }
        virtual ~OpenGLBuffer() { glDeleteBuffers(1, &Resource); }
    };
    using OpenGLBufferRef = std::shared_ptr<OpenGLBuffer>;

    class OpenGLUniformBuffer : public RHIUniformBuffer
    {
    public:
		GLuint	Resource;
		GLenum	Target;

        OpenGLUniformBuffer(
            uint32 InSize,
            EUniformBufferUsage InUsage,
            void *InData
        )
        : RHIUniformBuffer(InSize, InUsage)
        , Resource(0)
        , Target(GL_UNIFORM_BUFFER)
        {
            GLenum GLUsage = GL_STATIC_DRAW;
            switch (InUsage) 
            {
                case EUniformBufferUsage::UniformBuffer_SingleDraw:
                case EUniformBufferUsage::UniformBuffer_SingleFrame:
                    GLUsage = GL_DYNAMIC_DRAW;
                    break;
                case EUniformBufferUsage::UniformBuffer_MultiFrame:
                    GLUsage = GL_STATIC_DRAW; 
                    break;
            }
            glGenBuffers(1, &Resource);
            glBindBuffer(Target, Resource);
            glBufferData(Target, InSize, InData, GLUsage);
            glBindBuffer(Target, 0);
        }
        virtual ~OpenGLUniformBuffer() { glDeleteBuffers(1, &Resource); }
    };
    using OpenGLUniformBufferRef = std::shared_ptr<OpenGLUniformBuffer>;
    // class OpenGLShaderStorageBuffer : public TOpenGLBuffer<RHIShaderStorageBuffer>
    // {
    // public:
    //     OpenGLShaderStorageBuffer(
    //         uint32 InSize,
    //         void *InData,
    //         GLenum InUsage
    //     )
    //     : TOpenGLBuffer<RHIShaderStorageBuffer>(
    //         GL_SHADER_STORAGE_BUFFER,
    //         0,
    //         InSize,
    //         InData,
    //         InUsage)
    //     {}
    // };

    // class OpenGLAtomicCounterBuffer : public TOpenGLBuffer<RHIAtomicCounterBuffer>
    // {
    // public:
    //     OpenGLAtomicCounterBuffer(
    //         uint32 InValue,
    //         GLenum InUsage
    //     )
    //     : TOpenGLBuffer<RHIAtomicCounterBuffer>(
    //         GL_ATOMIC_COUNTER_BUFFER,
    //         0,
    //         4,
    //         &InValue,
    //         InUsage)
    //     {}
    // };

    struct DrawArraysIndirectCommand
    {
        GLsizei 	 Count;
        unsigned int instanceCount;
        unsigned int first;
        unsigned int baseInstance;
    };
    struct DrawElementsIndirectCommand
    {
        GLsizei 	 Count;
        unsigned int instanceCount;
        unsigned int firstIndex;
        unsigned int baseVertex;
        unsigned int baseInstance;
    };
    struct DispatchIndirectCommand {
        unsigned int num_groups_x;
        unsigned int num_groups_y;
        unsigned int num_groups_z;
    };

}