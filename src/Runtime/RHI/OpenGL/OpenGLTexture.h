#pragma once
#include <cstdint>
#include <string>
#include <memory>

#include <glad/glad.h>

#include "OpenGL/OpenGLTexture.h"
#include "Platform.h"
#include "RHIResources.h"

namespace nilou {
	class OpenGLTexture : public RHITexture
	{
	public:
		GLuint 			Resource;
		GLenum 			Target;

		/** The handle used for bindless texture */
		GLuint64		Handle;
		OpenGLTexture(
			GLuint InResource,
			GLenum InTarget,
			uint32 InSizeX,
			uint32 InSizeY,
			uint32 InSizeZ,
			uint32 InNumMips,
			EPixelFormat InFormat,
			const std::string &InTextureName,
			ETextureDimension InTextureType
		)
		: RHITexture(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName, InTextureType)
		, Resource(InResource)
		, Target(InTarget)
		{ }
		virtual ~OpenGLTexture() 
		{ 
			// glMakeTextureHandleNonResidentARB(Handle);
			glDeleteTextures(1, &Resource); 
		} 
	protected:
		class FOpenGLDynamicRHI *OpenGLRHI;

	};
	using OpenGLTextureRef = std::shared_ptr<OpenGLTexture>;

	// Deprecated typenames
	using OpenGLTexture2D = OpenGLTexture;
	using OpenGLTexture2DArray = OpenGLTexture;
	using OpenGLTexture3D = OpenGLTexture;
	using OpenGLTextureCube = OpenGLTexture;

	using OpenGLTexture2DRef = std::shared_ptr<OpenGLTexture2D>;
	using OpenGLTexture2DArrayRef = std::shared_ptr<OpenGLTexture2DArray>;
	using OpenGLTexture3DRef = std::shared_ptr<OpenGLTexture3D>;
	using OpenGLTextureCubeRef = std::shared_ptr<OpenGLTextureCube>;
}