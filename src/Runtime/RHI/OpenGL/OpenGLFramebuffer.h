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
		OpenGLFramebuffer(EFramebufferAttachment attachment, RHITexture2DRef texture);
		OpenGLFramebuffer(EFramebufferAttachment attachment, RHITexture2DArrayRef texture, unsigned int layer_index);
		virtual void AddAttachment(EFramebufferAttachment attachment, RHITexture2DRef texture) override;
		bool Check();
		virtual inline ~OpenGLFramebuffer() { glDeleteFramebuffers(1, &Resource); }
		virtual bool HasColorAttachment() override;
		virtual bool HasDepthAttachment() override;
		friend class FOpenGLDynamicRHI;
	private:
		std::vector<GLenum> m_ColorAttachments;
		GLint MaxAttach;
	};

	typedef std::shared_ptr<OpenGLFramebuffer> OpenGLFramebufferRef;
}