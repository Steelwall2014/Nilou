#pragma once
#include <glad/glad.h>
#include <memory>
#include <map>
#include <vector>

#include "RHIResources.h"
#include "OpenGL/OpenGLTexture.h"

namespace nilou {
	class OpenGLFramebuffer : public RHIFramebuffer
	{
	public:
		GLuint	Resource;
		OpenGLFramebuffer();
		void AddAttachment(EFramebufferAttachment attachment, RHITexture2DRef texture);
		bool Check();
		virtual ~OpenGLFramebuffer() { glDeleteFramebuffers(1, &Resource); }
		friend class FOpenGLDynamicRHI;
	private:
		std::vector<GLenum> m_ColorAttachments;
		GLint MaxAttach;
	};

	typedef std::shared_ptr<OpenGLFramebuffer> OpenGLFramebufferRef;
}