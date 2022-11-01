#pragma once
#include <vector>
#include <map>
#include "glad/glad.h"
#include "RHIResources.h"

namespace und {
	class OpenGLVertexArrayObject : public RHIVertexArrayObject
	{
		friend class FOpenGLDynamicRHI;
	public:
		OpenGLVertexArrayObject(EPrimitiveMode InMode);
		virtual void AddVertexAttribBuffer(const RHIShaderResourceView &SRV, RHIBufferRef VertexAttri);
		virtual void SetIndexBuffer(RHIBufferRef IndexBuffer);
		virtual ~OpenGLVertexArrayObject();
		GLenum GetGLPrimitiveMode();
		/**
		* @return the count used in glDrawElements or glDrawArrays
		*/
		virtual int32 GetPointCount() override;
		GLenum GetIndexBufferGLDataType();
	private:
		bool	m_VertexArrayInitialized;
		GLuint	m_VAO;
		GLint	m_Count;  
	};

	typedef std::shared_ptr<OpenGLVertexArrayObject> OpenGLVertexArrayObjectRef;
}