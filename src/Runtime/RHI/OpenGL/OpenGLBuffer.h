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

    // class OpenGLDrawArraysIndirectBuffer : public TOpenGLBuffer<RHIDrawArraysIndirectBuffer>
    // {
    // public:
    //     OpenGLDrawArraysIndirectBuffer(DrawArraysIndirectCommand IndirectCmd, GLenum InUsage)
    //     : TOpenGLBuffer<RHIDrawArraysIndirectBuffer>(
    //         GL_DRAW_INDIRECT_BUFFER,
    //         0,
    //         sizeof(IndirectCmd),
    //         &IndirectCmd,
    //         InUsage)
    //     {}
    // };

    // class OpenGLDrawElementsIndirectBuffer : public TOpenGLBuffer<RHIDrawElementsIndirectBuffer>
    // {
    // public:
    //     OpenGLDrawElementsIndirectBuffer(DrawElementsIndirectCommand IndirectCmd, GLenum InUsage)
    //     : TOpenGLBuffer<RHIDrawElementsIndirectBuffer>(
    //         GL_DRAW_INDIRECT_BUFFER,
    //         0,
    //         sizeof(IndirectCmd),
    //         &IndirectCmd,
    //         InUsage)
    //     {}
    // };

    // class OpenGLDispatchIndirectBuffer : public TOpenGLBuffer<RHIDipatchIndirectBuffer>
    // {
    // public:
    //     OpenGLDispatchIndirectBuffer(DispatchIndirectCommand IndirectCmd, GLenum InUsage)
    //     : TOpenGLBuffer<RHIDipatchIndirectBuffer>(
    //         GL_DISPATCH_INDIRECT_BUFFER,
    //         0,
    //         sizeof(IndirectCmd),
    //         &IndirectCmd,
    //         InUsage)
    //     {}
    // };

	// class OpenGLComputeBuffer : public OpenGLBaseBuffer
	// {
	// public:
	// 	OpenGLComputeBuffer(GLenum target, unsigned int DataByteLength, void *Data, GLenum usage=GL_DYNAMIC_DRAW);
	// };
	// typedef std::shared_ptr<OpenGLComputeBuffer> OpenGLComputeBufferRef;

	// class OpenGLShaderStorageBuffer : public OpenGLComputeBuffer
	// {
	// public:
	// 	OpenGLShaderStorageBuffer(unsigned int DataByteLength, void *Data, GLenum usage = GL_DYNAMIC_DRAW)
	// 		: OpenGLComputeBuffer(GL_SHADER_STORAGE_BUFFER, DataByteLength, Data, usage)
	// 	{}
	// };
	// typedef std::shared_ptr<OpenGLShaderStorageBuffer> OpenGLShaderStorageBufferRef;

	// class OpenGLAtomicCounterBuffer : public OpenGLComputeBuffer
	// {
	// public:
	// 	OpenGLAtomicCounterBuffer(unsigned int value, GLenum usage = GL_DYNAMIC_DRAW)
	// 		: OpenGLComputeBuffer(GL_ATOMIC_COUNTER_BUFFER, 4, &value, usage)
	// 	{}
	// };
	// typedef std::shared_ptr<OpenGLAtomicCounterBuffer> OpenGLAtomicCounterBufferRef;


	// class OpenGLIndexArrayBuffer : public OpenGLBaseBuffer
	// {
	// public:
	// 	// glBufferData(GL_ELEMENT_BUFFER, DataByteSize, Data, Usage)
	// 	GLsizeiptr		DataByteSize;
	// 	//void *			Data;
	// 	GLsizei			Count;
	// 	GLenum			Usage;
	// 	GLenum			DataType;
	// 	//OpenGLIndexArrayBuffer(int Count, GLenum DataType, GLenum Usage = GL_STATIC_DRAW);
	// 	OpenGLIndexArrayBuffer(int DataByteSize, void *Data, GLenum DataType, int Count=0, GLenum Usage = GL_STATIC_DRAW);
	// 	//virtual ~OpenGLIndexArrayBuffer();
	// 	friend class FOpenGLDynamicRHI;
	// };
	// typedef std::shared_ptr<OpenGLIndexArrayBuffer> OpenGLIndexArrayBufferRef;

	// class OpenGLVertexAttribBuffer : public OpenGLBaseBuffer
	// {
	// public:
	// 	// glBufferData(GL_ARRAY_BUFFER, DataByteSize, Data, Usage)
	// 	GLsizeiptrARB	DataByteSize;
	// 	//void *			Data;
	// 	GLenum			Usage;

	// 	// glVertexAttribPointer(<index>, ElemSize, DataType, Normalize, Stride, Offset)
	// 	GLint			ElemSize;
	// 	GLenum			DataType;
	// 	GLsizei			Stride;
	// 	bool			Normalize;
	// 	size_t			Offset;

	// 	//OpenGLVertexAttribBuffer(int ElemNum, GLint ElemSize, GLenum DataType, bool Normalize=false, size_t Offset=0, GLenum Usage=GL_STATIC_DRAW);
	// 	OpenGLVertexAttribBuffer(int DataByteSize, void *Data, GLint ElemSize, GLenum DataType, GLsizei Stride=0, bool Normalize=false, size_t Offset=0, GLenum Usage = GL_STATIC_DRAW);
	// 	//virtual ~OpenGLVertexAttribBuffer();

	// 	friend class FOpenGLDynamicRHI;
	// };
	// typedef std::shared_ptr<OpenGLVertexAttribBuffer> OpenGLVertexAttribBufferRef;
	// //typedef std::shared_ptr<OpenGLVertexAttribBuffer<T>> OpenGLVertexAttribBufferRef<T>

	// class OpenGLIndirectBuffer : public OpenGLBaseBuffer
	// {
	// public:
	// 	struct DrawArraysIndirectCommand
	// 	{
	// 		GLsizei 	 Count;
	// 		unsigned int instanceCount;
	// 		unsigned int first;
	// 		unsigned int baseInstance;
	// 	};
	// 	struct DrawElementsIndirectCommand
	// 	{
	// 		GLsizei 	 Count;
	// 		unsigned int instanceCount;
	// 		unsigned int firstIndex;
	// 		unsigned int baseVertex;
	// 		unsigned int baseInstance;
	// 	};
	// 	struct DispatchIndirectCommand {
	// 		unsigned int num_groups_x;
	// 		unsigned int num_groups_y;
	// 		unsigned int num_groups_z;
	// 	};
	// 	OpenGLIndirectBuffer(GLenum Target, unsigned int DataByteLength, void *Data, GLenum usage = GL_DYNAMIC_DRAW)
	// 		: OpenGLBaseBuffer(Target, DataByteLength, Data, usage)
	// 	{
	// 	}
	// };
	// typedef std::shared_ptr<OpenGLIndirectBuffer> OpenGLIndirectBufferRef;

	// class OpenGLDrawElementsIndirectBuffer : public OpenGLIndirectBuffer
	// {
	// public:
	// 	OpenGLDrawElementsIndirectBuffer(DrawElementsIndirectCommand initCommand, GLenum usage = GL_DYNAMIC_DRAW)
	// 		: OpenGLIndirectBuffer(GL_DRAW_INDIRECT_BUFFER, sizeof(initCommand), &initCommand, usage)
	// 	{
	// 	}
	// };
	// typedef std::shared_ptr<OpenGLDrawElementsIndirectBuffer> OpenGLDrawElementsIndirectBufferRef;

	// class OpenGLDrawArraysIndirectBuffer : public OpenGLIndirectBuffer
	// {
	// public:
	// 	OpenGLDrawArraysIndirectBuffer(DrawArraysIndirectCommand initCommand, GLenum usage = GL_DYNAMIC_DRAW)
	// 		: OpenGLIndirectBuffer(GL_DRAW_INDIRECT_BUFFER, sizeof(initCommand), &initCommand, usage)
	// 	{
	// 	}
	// };
	// typedef std::shared_ptr<OpenGLDrawArraysIndirectBuffer> OpenGLDrawArraysIndirectBufferRef;

	// class OpenGLDipatchIndirectBuffer : public OpenGLIndirectBuffer
	// {
	// public:
	// 	OpenGLDipatchIndirectBuffer(DispatchIndirectCommand initCommand, GLenum usage = GL_DYNAMIC_DRAW)
	// 		: OpenGLIndirectBuffer(GL_DISPATCH_INDIRECT_BUFFER, sizeof(initCommand), &initCommand, usage)
	// 	{
	// 	}
	// };
	// typedef std::shared_ptr<OpenGLDipatchIndirectBuffer> OpenGLDipatchIndirectBufferRef;

}