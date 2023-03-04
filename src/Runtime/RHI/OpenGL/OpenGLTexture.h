#pragma once
#include <cstdint>
#include <string>
#include <memory>

#include <glad/glad.h>

#include "OpenGL/OpenGLTexture.h"
#include "Platform.h"
#include "RHIResources.h"

namespace nilou {
	class OpenGLTextureBase
	{
	public:
		GLuint 			Resource;
		GLenum 			Target;
		OpenGLTextureBase(
			GLuint InResource,
			GLenum InTarget
		)
		: Resource(InResource)
		, Target(InTarget)
		{ }
		virtual ~OpenGLTextureBase() { glDeleteTextures(1, &Resource); } 
	protected:
		class FOpenGLDynamicRHI *OpenGLRHI;

	};

	template<typename BaseType>
	class TOpenGLTexture : public BaseType, public OpenGLTextureBase
	{
	public:
		TOpenGLTexture(
			GLuint InResource,
			GLenum InTarget,
			uint32 InSizeX,
			uint32 InSizeY,
			uint32 InSizeZ,
			uint32 InNumMips,
			EPixelFormat InFormat,
			const std::string &InTextureName
		)
		: OpenGLTextureBase(InResource, InTarget)
		, BaseType(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
		{}
	};

	class OpenGLBaseTexture : public RHITexture
	{
	public:
		OpenGLBaseTexture(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
		: RHITexture(InNumMips, InFormat, InTextureName)
		{ }
	};

	class OpenGLBaseTexture2D : public RHITexture2D
	{
	public:
		OpenGLBaseTexture2D(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
		: RHITexture2D(InSizeX, InSizeY, InNumMips, InFormat, InTextureName)
		{ }
	};

	class OpenGLBaseTexture2DArray : public RHITexture2DArray
	{
	public:
		OpenGLBaseTexture2DArray(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
		: RHITexture2DArray(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
		{ }
	};

	class OpenGLBaseTexture3D : public RHITexture3D
	{
	public:
		OpenGLBaseTexture3D(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
		: RHITexture3D(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
		{ }
	};

	class OpenGLBaseTextureCube : public RHITextureCube
	{
	public:
		OpenGLBaseTextureCube(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
		: RHITextureCube(InSizeX, InNumMips, InFormat, InTextureName)
		{ }
	};
	using OpenGLTexture = TOpenGLTexture<OpenGLBaseTexture>;
	using OpenGLTexture2D = TOpenGLTexture<OpenGLBaseTexture2D>;
	using OpenGLTexture2DArray = TOpenGLTexture<OpenGLBaseTexture2DArray>;
	using OpenGLTexture3D = TOpenGLTexture<OpenGLBaseTexture3D>;
	using OpenGLTextureCube = TOpenGLTexture<OpenGLBaseTextureCube>;

	using OpenGLTextureBaseRef = std::shared_ptr<OpenGLTextureBase>;
	using OpenGLTextureRef = std::shared_ptr<OpenGLTexture>;
	using OpenGLTexture2DRef = std::shared_ptr<OpenGLTexture2D>;
	using OpenGLTexture2DArrayRef = std::shared_ptr<OpenGLTexture2DArray>;
	using OpenGLTexture3DRef = std::shared_ptr<OpenGLTexture3D>;
	using OpenGLTextureCubeRef = std::shared_ptr<OpenGLTextureCube>;
}