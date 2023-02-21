#include "DynamicRHI.h"
#include "UniformBuffer.h"

namespace nilou {
	RHITextureParams RHITextureParams::DefaultParams = RHITextureParams();

	FDynamicRHI *FDynamicRHI::DynamicRHI = nullptr;

	FDynamicRHI *FDynamicRHI::GetDynamicRHI()
	{
		return FDynamicRHI::DynamicRHI;
	}
}