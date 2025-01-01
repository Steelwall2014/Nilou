#pragma once
#include "Platform.h"
    
namespace nilou {

	enum EPixelFormat : uint8
	{
		PF_Unknown = 0,
		PF_R8,
		PF_R8UI,
		PF_R8G8,
		PF_R8G8B8,
		PF_R8G8B8_sRGB,
		PF_B8G8R8,
		PF_B8G8R8_sRGB,
		PF_R8G8B8A8,
		PF_R8G8B8A8_sRGB,
		PF_B8G8R8A8,
		PF_B8G8R8A8_sRGB,

		PF_D24S8,
		PF_D32F,
		PF_D32FS8,

		PF_DXT1,
		PF_DXT1_sRGB,
		PF_DXT5,
		PF_DXT5_sRGB,

		PF_R16F,
		PF_R16G16F,
		PF_R16G16B16F,
		PF_R16G16B16A16F,
		PF_R32F,
		PF_R32G32F,
		PF_R32G32B32F,
		PF_R32G32B32A32F,

		PF_MAX
	};
	uint8 TranslatePixelFormatToBytePerPixel(EPixelFormat PixelFormat);
	uint8 TranslatePixelFormatToChannel(EPixelFormat PixelFormat);
	inline bool IsStencilFormat(EPixelFormat PixelFormat)
	{
		return PixelFormat == PF_D24S8 || PixelFormat == PF_D32FS8;
	}

	struct FPixelFormatInfo
	{
		FPixelFormatInfo() = delete;
		FPixelFormatInfo(
			EPixelFormat InFormat,
			const std::string& InName,
			int32 InBlockSizeX,
			int32 InBlockSizeY,
			int32 InBlockSizeZ,
			int32 InBlockBytes,
			int32 InNumComponents);

		std::string					Name;
		EPixelFormat				Format;
		int32						BlockSizeX;
		int32						BlockSizeY;
		int32						BlockSizeZ;
		int32						BlockBytes;
		int32						NumComponents;

		/** Platform specific converted format (initialized by RHI module - invalid otherwise) */
		uint32						PlatformFormat{ 0 };

		// If false, 32 bit float is assumed (initialized by RHI module - invalid otherwise)
		uint8						bIs24BitUnormDepthStencil : 1;
	};

	extern FPixelFormatInfo GPixelFormats[PF_MAX];		// Maps members of EPixelFormat to a FPixelFormatInfo describing the format.

}