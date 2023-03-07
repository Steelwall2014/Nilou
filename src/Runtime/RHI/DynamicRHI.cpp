#include "DynamicRHI.h"
#include "UniformBuffer.h"

namespace nilou {
	RHITextureParams RHITextureParams::DefaultParams = RHITextureParams();

	FDynamicRHI *FDynamicRHI::DynamicRHI = nullptr;

	FDynamicRHI *FDynamicRHI::GetDynamicRHI()
	{
		return FDynamicRHI::DynamicRHI;
	}

	ivec3 FDynamicRHI::RHIGetSparseTexturePageSize(ETextureType TextureType, EPixelFormat PixelFormat) 
	{ 
		return FDynamicRHI::SparseTextureTileSizes[(int)TextureType][(int)PixelFormat]; 
	}

	ivec3 FDynamicRHI::SparseTextureTileSizes[(int)ETextureType::TT_TextureTypeNum][(int)EPixelFormat::PF_PixelFormatNum];
}