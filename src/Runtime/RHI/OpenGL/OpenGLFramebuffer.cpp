#include "OpenGL/OpenGLFramebuffer.h"
#include "OpenGL/OpenGLTexture.h"
#include <memory>

namespace nilou {

	GLenum TranslateFramebufferAttachment(EFramebufferAttachment Attachment)
	{
		#define ATTACHMENT(N) \
			case EFramebufferAttachment::FA_Color_Attachment##N: \
				return GL_COLOR_ATTACHMENT##N; 

		switch (Attachment) {
			ATTACHMENT(0)
			ATTACHMENT(1)
			ATTACHMENT(2)
			ATTACHMENT(3)
			ATTACHMENT(4)
			ATTACHMENT(5)
			ATTACHMENT(6)
			ATTACHMENT(7)
			// ATTACHMENT(8)
			// ATTACHMENT(9)
			// ATTACHMENT(10)
			// ATTACHMENT(12)
			// ATTACHMENT(13)
			// ATTACHMENT(14)
			// ATTACHMENT(15)
			// ATTACHMENT(16)
			// ATTACHMENT(17)
			// ATTACHMENT(18)
			// ATTACHMENT(19)
			// ATTACHMENT(20)
			// ATTACHMENT(21)
			// ATTACHMENT(22)
			// ATTACHMENT(23)
			// ATTACHMENT(24)
			// ATTACHMENT(25)
			// ATTACHMENT(26)
			// ATTACHMENT(27)
			// ATTACHMENT(28)
			// ATTACHMENT(29)
			// ATTACHMENT(30)
			// ATTACHMENT(31)
			// case EFramebufferAttachment::FA_Depth_Attachment:
			// 	return GL_DEPTH_ATTACHMENT;
			// case EFramebufferAttachment::FA_Stencil_Attachment:
			// 	return GL_STENCIL_ATTACHMENT; 
			case EFramebufferAttachment::FA_Depth_Stencil_Attachment:
				return GL_DEPTH_STENCIL_ATTACHMENT;
		}

		#undef ATTACHMENT
	}

	OpenGLFramebuffer::OpenGLFramebuffer()
	{
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MaxAttach);
		glGenFramebuffers(1, &Resource);
	}
	void OpenGLFramebuffer::AddAttachment(EFramebufferAttachment attachment, RHITexture2DRef texture)
	{
		GLenum GLAttachment = TranslateFramebufferAttachment(attachment);
		OpenGLTexture2DRef GLTexture = std::static_pointer_cast<OpenGLTexture2D>(texture);
		glBindFramebuffer(GL_FRAMEBUFFER, Resource);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GLAttachment, GLTexture->Target, GLTexture->Resource, 0);
		glDrawBuffers(m_ColorAttachments.size(), m_ColorAttachments.data());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Attachments[attachment] = texture;
		if (GL_COLOR_ATTACHMENT0 <= GLAttachment && GLAttachment <= GL_COLOR_ATTACHMENT0+MaxAttach)
			m_ColorAttachments.push_back(GLAttachment);
	}
	bool OpenGLFramebuffer::Check()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, Resource);
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return status == GL_FRAMEBUFFER_COMPLETE;
	}
}
