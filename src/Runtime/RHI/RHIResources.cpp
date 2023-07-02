#pragma once

#include "RHIResources.h"
#include "Common/Log.h"
#include "RHIStaticStates.h"
#include "Common/Crc.h"

namespace nilou {

    int FRHIGraphicsPipelineState::GetBaseIndexByName(EPipelineStage PipelineStage, const std::string &Name)
    {
        FRHIDescriptorSet &DescriptorSet = PipelineLayout->DescriptorSets[PipelineStage];
        auto iter = DescriptorSet.Bindings.find(Name);
        if (iter != DescriptorSet.Bindings.end())
            return iter->second.BindingPoint;
        else
        {
            NILOU_LOG(Error, "Shader Parameter {} Not found", Name.c_str())
            return -1;
        }
    }

    uint8 TranslatePixelFormatToBytePerPixel(EPixelFormat PixelFormat)
    {
        switch (PixelFormat) {
		    case EPixelFormat::PF_UNKNOWN: return 0;
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
		    case EPixelFormat::PF_UNKNOWN: return 0;
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

	bool FGraphicsPipelineStateInitializer::operator==(const FGraphicsPipelineStateInitializer &Other) const
	{
		return 	RenderTargetFormats == Other.RenderTargetFormats && 
				NumRenderTargetsEnabled == Other.NumRenderTargetsEnabled && 
				DepthStencilTargetFormat == Other.DepthStencilTargetFormat &&
				VertexShader == Other.VertexShader &&
				PixelShader == Other.PixelShader &&
				ComputeShader == Other.ComputeShader && 
				PrimitiveMode == Other.PrimitiveMode && 
				DepthStencilState == Other.DepthStencilState && 
				RasterizerState == Other.RasterizerState && 
				BlendState == Other.BlendState && 
				VertexDeclaration == Other.VertexDeclaration;
	}
	
	void FGraphicsPipelineStateInitializer::BuildRenderTargetFormats(RHIFramebuffer* Framebuffer)
	{
		NumRenderTargetsEnabled = 0;
		if (Framebuffer)
		{
			for (auto [Attachment, Texture] : Framebuffer->Attachments)
			{
				if (Attachment == EFramebufferAttachment::FA_Depth_Stencil_Attachment)
				{
					DepthStencilTargetFormat = Texture->GetFormat();
				}
				else 
				{
					uint32 index = (uint8)Attachment-(uint8)EFramebufferAttachment::FA_Color_Attachment0;
					RenderTargetFormats[index] = Texture->GetFormat();
					NumRenderTargetsEnabled = std::max(NumRenderTargetsEnabled, index+1);
				}
			}
		}
	}
	
	FRHISampler::FRHISampler()
		: SamplerState(TStaticSamplerState<>::CreateRHI().get())
		, Texture(nullptr) 
	{ }

	
	FRHISampler::FRHISampler(RHITexture* Texture)
		: SamplerState(TStaticSamplerState<>::CreateRHI().get())
		, Texture(Texture) 
	{ }

}

namespace std {

size_t hash<nilou::FGraphicsPipelineStateInitializer>::operator()(const nilou::FGraphicsPipelineStateInitializer &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

}