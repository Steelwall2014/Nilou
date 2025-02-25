#pragma once

#include "RHIResources.h"
#include "Common/Log.h"
#include "RHIStaticStates.h"
#include "Common/Crc.h"

namespace nilou {

	RHITextureView* FRHITextureViewCache::GetOrCreateView(RHITexture* Texture, const FRHITextureViewCreateInfo& InCreateInfo)
	{
		for (auto& [Desc, View] : TextureViews)
		{
			if (Desc == InCreateInfo)
			{
				return View;
			}
		}

		RHITextureViewRef TextureView = RHICreateTextureView(Texture, InCreateInfo, Texture->GetName() + "_View");
		TextureViews.push_back({InCreateInfo, TextureView});
		return TextureView;
	}

    uint8 TranslatePixelFormatToBytePerPixel(EPixelFormat PixelFormat)
    {
        switch (PixelFormat) {
		    case EPixelFormat::PF_Unknown: return 0;
		    case EPixelFormat::PF_R8: return 1;
		    case EPixelFormat::PF_R8UI: return 1;
		    case EPixelFormat::PF_R8G8: return 2;
		    case EPixelFormat::PF_R8G8B8: return 3;
		    case EPixelFormat::PF_R8G8B8_sRGB: return 3;
		    case EPixelFormat::PF_B8G8R8: return 3;
		    case EPixelFormat::PF_B8G8R8_sRGB: return 3;
		    case EPixelFormat::PF_R8G8B8A8: return 4;
		    case EPixelFormat::PF_R8G8B8A8_sRGB: return 4;
		    case EPixelFormat::PF_B8G8R8A8: return 4;
		    case EPixelFormat::PF_B8G8R8A8_sRGB: return 4;

		    case EPixelFormat::PF_D24S8: return 4;
		    case EPixelFormat::PF_D32F: return 4;
		    case EPixelFormat::PF_D32FS8: return 5;

		    case EPixelFormat::PF_DXT1: return 4;
		    case EPixelFormat::PF_DXT1_sRGB: return 4;
		    case EPixelFormat::PF_DXT5: return 4;
		    case EPixelFormat::PF_DXT5_sRGB: return 4;

		    case EPixelFormat::PF_R16F: return 2;
		    case EPixelFormat::PF_R16G16F: return 4;
		    case EPixelFormat::PF_R16G16B16F: return 6;
		    case EPixelFormat::PF_R16G16B16A16F: return 8;
		    case EPixelFormat::PF_R32F: return 4;
		    case EPixelFormat::PF_R32G32F: return 8;
		    case EPixelFormat::PF_R32G32B32F: return 12;
		    case EPixelFormat::PF_R32G32B32A32F: return 16;
            default: NILOU_LOG(Error, "Unknown PixelFormat: {}", (int)PixelFormat) return 0;
        }
    }

    uint8 TranslatePixelFormatToChannel(EPixelFormat PixelFormat)
    {
        switch (PixelFormat) {
		    case EPixelFormat::PF_Unknown: return 0;
		    case EPixelFormat::PF_R8: return 1;
		    case EPixelFormat::PF_R8UI: return 1;
		    case EPixelFormat::PF_R8G8: return 2;
		    case EPixelFormat::PF_R8G8B8: return 3;
		    case EPixelFormat::PF_R8G8B8_sRGB: return 3;
		    case EPixelFormat::PF_B8G8R8: return 3;
		    case EPixelFormat::PF_B8G8R8_sRGB: return 3;
		    case EPixelFormat::PF_R8G8B8A8: return 4;
		    case EPixelFormat::PF_R8G8B8A8_sRGB: return 4;
		    case EPixelFormat::PF_B8G8R8A8: return 4;
		    case EPixelFormat::PF_B8G8R8A8_sRGB: return 4;

		    case EPixelFormat::PF_D24S8: return 1;
		    case EPixelFormat::PF_D32F: return 1;
		    case EPixelFormat::PF_D32FS8: return 1;

		    case EPixelFormat::PF_DXT1: return 3;
		    case EPixelFormat::PF_DXT1_sRGB: return 3;
		    case EPixelFormat::PF_DXT5: return 4;
		    case EPixelFormat::PF_DXT5_sRGB: return 4;

		    case EPixelFormat::PF_R16F: return 1;
		    case EPixelFormat::PF_R16G16F: return 2;
		    case EPixelFormat::PF_R16G16B16F: return 3;
		    case EPixelFormat::PF_R16G16B16A16F: return 4;
		    case EPixelFormat::PF_R32F: return 1;
		    case EPixelFormat::PF_R32G32F: return 2;
		    case EPixelFormat::PF_R32G32B32F: return 3;
		    case EPixelFormat::PF_R32G32B32A32F: return 4;
            default: NILOU_LOG(Error, "Unknown PixelFormat: {}", (int)PixelFormat) return 0;
        }
    }

	void RHIDescriptorSetLayout::GenerateHash()
	{
		Hash = FCrc::MemCrc32(Bindings.data(), sizeof(RHIDescriptorSetLayoutBinding) * Bindings.size());
	}

}

namespace std {

size_t hash<nilou::FGraphicsPipelineStateInitializer>::operator()(const nilou::FGraphicsPipelineStateInitializer &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

}