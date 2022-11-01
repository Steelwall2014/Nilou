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
		GLenum			Attachment;
		OpenGLTextureBase(
			class FOpenGLDynamicRHI* InOpenGLRHI,
			GLuint InResource,
			GLenum InTarget,
			GLenum InAttachment
		)
		: OpenGLRHI(InOpenGLRHI)
		, Resource(InResource)
		, Target(InTarget)
		, Attachment(InAttachment)
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
			class FOpenGLDynamicRHI* InOpenGLRHI,
			GLuint InResource,
			GLenum InTarget,
			GLenum InAttachment,
			uint32 InSizeX,
			uint32 InSizeY,
			uint32 InSizeZ,
			uint32 InNumMips,
			EPixelFormat InFormat,
			const std::string &InTextureName
		)
		: OpenGLTextureBase(InOpenGLRHI, InResource, InTarget, InAttachment)
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

	// class OpenGLTexture : public RHITexture
	// {
	// 	friend class FOpenGLDynamicRHI;
	// 	friend class OpenGLFramebuffer;
	// public:
	// 	std::string		TextureName;
	// 	unsigned int	Width;
	// 	unsigned int	Height;
	// 	unsigned int	BytesPerPixel;
	// 	GLenum			PixelFormat;
	// protected:
	// 	GLuint			m_Texture;
	// 	GLenum			m_Target;
	// 	GLenum			m_Format;
	// 	GLenum			m_Type;
	// 	bool			m_MipmapGenerated;
	// 	OpenGLTexture(
	// 		std::string TextureName, GLenum PixelFormat,
	// 		GLenum Target, unsigned int Width, unsigned int Height);
	// public:
	// 	virtual ~OpenGLTexture() { glDeleteTextures(1, &m_Texture); }
	// };

	// class OpenGLTexture2D : public OpenGLTexture
	// {
	// protected:
	// 	OpenGLTexture2D(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height);
	// public:
	// 	OpenGLTexture2D(
	// 		std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, void *data);
	// };

	// class OpenGLTexture3D : public OpenGLTexture
	// {
	// protected:
	// 	OpenGLTexture3D(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int depth);
	// public:
	// 	unsigned int Depth;
	// 	OpenGLTexture3D(
	// 		std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int depth, void *data);
	// };

	// class OpenGLTextureCube : public OpenGLTexture
	// {
	// public:
	// 	OpenGLTextureCube(
	// 		std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, void *data[6]);
	// };

	// class OpenGLTexture2DArray : public OpenGLTexture
	// {
	// public:
	// 	OpenGLTexture2DArray(
	// 		std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int layer_count, void *data);
	// 	unsigned int LayerCount;
	// };

	// class OpenGLTextureImage2D : public OpenGLTexture2D
	// {
	// 	friend class OceanSurfacePass;
	// public:
	// 	OpenGLTextureImage2D(
	// 		std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, bool mipmap=false);
	// };

	// class OpenGLTextureImage3D : public OpenGLTexture3D
	// {
	// public:
	// 	OpenGLTextureImage3D(
	// 		std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int depth, bool mipmap = false);
	// };
	// using OpenGLTextureRef = std::shared_ptr<OpenGLTexture>;
	// using OpenGLTexture2DRef = std::shared_ptr<OpenGLTexture2D>;
	// using OpenGLTexture3DRef = std::shared_ptr<OpenGLTexture3D>;
	// using OpenGLTextureCubeRef = std::shared_ptr<OpenGLTextureCube>;
	// using OpenGLTexture2DArrayRef = std::shared_ptr<OpenGLTexture2DArray>;
	// using OpenGLTextureImage2DRef = std::shared_ptr<OpenGLTextureImage2D>;
	// using OpenGLTextureImage3DRef = std::shared_ptr<OpenGLTextureImage3D>;
}