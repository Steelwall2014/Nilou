#include <iostream>
#include "OpenGLVertexArrayObject.h"

namespace und {
	OpenGLVertexArrayObject::OpenGLVertexArrayObject(EPrimitiveMode InMode)
		: RHIVertexArrayObject(InMode)
	{
		glGenVertexArrays(1, &m_VAO);
		m_VertexArrayInitialized = false;
	}


	void OpenGLVertexArrayObject::AddVertexAttribBuffer(const RHIShaderResourceView &SRV, RHIBufferRef VertexAttri)
	{
		VertexAttribs[SRV] = VertexAttri;
		if (IndexBuffer == nullptr)
			m_Count = VertexAttri->GetSize() / VertexAttri->GetStride();
	}


	void OpenGLVertexArrayObject::SetIndexBuffer(RHIBufferRef IndexBuffer)
	{
		this->IndexBuffer = IndexBuffer;
		m_Count = IndexBuffer->GetSize() / IndexBuffer->GetStride();
	}


	OpenGLVertexArrayObject::~OpenGLVertexArrayObject()
	{
		glDeleteVertexArrays(1, &m_VAO);
	}


	GLenum OpenGLVertexArrayObject::GetGLPrimitiveMode()
	{
		switch (this->Mode) 
		{
		case EPrimitiveMode::PM_Triangle_Strip:
			return GL_TRIANGLE_STRIP;
		case EPrimitiveMode::PM_Triangles:
			return GL_TRIANGLES;
		}
	}

	int32 OpenGLVertexArrayObject::GetPointCount()
	{
		return m_Count;
	}

	GLenum OpenGLVertexArrayObject::GetIndexBufferGLDataType()
	{
		GLenum data_type = 0;
		if (IndexBuffer == nullptr)
			return 0;
		switch (IndexBuffer->GetStride()) 
		{
			case 1: data_type = GL_UNSIGNED_BYTE; break;
			case 2: data_type = GL_UNSIGNED_SHORT; break;
			case 4: data_type = GL_UNSIGNED_INT; break;
			default: std::cout << "Invalid data type for index buffer"; break;
		}
		return data_type;
	}
}