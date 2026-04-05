#pragma once

#include "RHIResources.h"
#include "Logging/LogMacros.h"
#include "Misc/Crc.h"
#include "DynamicRHI.h"

namespace nilou {

	RHITextureView* FRHITextureViewCache::GetOrCreateView(RHITexture* Texture, const FRHITextureViewCreateInfo& InCreateInfo)
	{
		for (auto& [Desc, View] : TextureViews)
		{
			if (Desc == InCreateInfo)
			{
				return View.GetReference();
			}
		}

		RHITextureViewRef TextureView = RHICreateTextureView(Texture, InCreateInfo, Texture->GetName() + "_View");
		TextureViews.push_back({InCreateInfo, TextureView});
		return TextureView.GetReference();
	}

	RHIDepthStencilState* GetDefaultDepthStencilState()
	{
		static RHIDepthStencilStateRef DefaultDepthStencilState = RHICreateDepthStencilState(FDepthStencilStateInitializer());
		return DefaultDepthStencilState.GetReference();
	}

	RHIRasterizerState* GetDefaultRasterizerState()
	{
		static RHIRasterizerStateRef DefaultRasterizerState = RHICreateRasterizerState(FRasterizerStateInitializer());
		return DefaultRasterizerState.GetReference();
	}

	RHIBlendState* GetDefaultBlendState()
	{
		static RHIBlendStateRef DefaultBlendState = RHICreateBlendState(FBlendStateInitializer());
		return DefaultBlendState.GetReference();
	}

	FGraphicsPipelineStateInitializer::FGraphicsPipelineStateInitializer()
		: DepthStencilState(GetDefaultDepthStencilState())
		, RasterizerState(GetDefaultRasterizerState())
		, BlendState(GetDefaultBlendState())
		, VertexDeclaration(nullptr)
		, PrimitiveMode(EPrimitiveMode::PM_TriangleList)
	{ 

	}

	RHIDescriptorSetLayout* RHIDescriptorSet::GetLayout() const
	{
		return Pool->Layout;
	}
}

namespace std {

size_t hash<nilou::FGraphicsPipelineStateInitializer>::operator()(const nilou::FGraphicsPipelineStateInitializer &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

}