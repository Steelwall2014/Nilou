#include "Vulkan/VulkanDynamicRHI.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "UniformBuffer.h"
#include "PipelineStateCache.h"

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

	void FDynamicRHI::CreateDynamicRHI_RenderThread(const GfxConfiguration& configs)
	{
		if (!strcmp(configs.defaultRHI, "opengl"))
		{
			FDynamicRHI::DynamicRHI = new FOpenGLDynamicRHI;
		}
		else if (!strcmp(configs.defaultRHI, "vulkan"))
		{
			FDynamicRHI::DynamicRHI = new FVulkanDynamicRHI;
		}
	}

	void FDynamicRHI::Finalize()
	{
		FPipelineStateCache::ClearCacheGraphicsPSO();
	}

	ivec3 FDynamicRHI::SparseTextureTileSizes[(int)ETextureType::TT_TextureTypeNum][(int)EPixelFormat::PF_PixelFormatNum];
}