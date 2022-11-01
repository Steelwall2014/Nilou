#pragma once

#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RHI.h"

namespace und {

    void RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, int32 BaseIndex, RHIUniformBuffer *);
	void RHISetShaderSampler(FRHIGraphicsPipelineState *, int32 BaseIndex, FRHISampler *);
	void RHISetShaderImage(FRHIGraphicsPipelineState *, int32 BaseIndex, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly);
	void RHISetVertexBuffer(FRHIGraphicsPipelineState *, FRHIVertexInput *);
	FRHIGraphicsPipelineState *RHIGetBoundPipelineState();

	FRHIGraphicsPipelineState *RHIGetOrCreatePipelineStateObject(const FRHIGraphicsPipelineStateData &StateData);

	void RHISetRasterizerState(RHIRasterizerState *newState);
	void RHISetDepthStencilState(RHIDepthStencilState *newState, uint32 StencilRef=0);
	void RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState);

	RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer);
	RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer);
	FRHIGraphicsPipelineStateRef RHICreateGraphicsPipelineState(const FRHIGraphicsPipelineStateData &NewState);

	RHIVertexShaderRef RHICreateVertexShader(const char *code);
	RHIPixelShaderRef RHICreatePixelShader(const char *code);
	RHIComputeShaderRef RHICreateComputeShader(const char *code);

	RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data);
	RHIUniformBufferRef RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data);
		
	void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data);
    /**
    * 暂时只支持一个属性一个buffer
    */
	RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data);
	RHIBufferRef RHICreateAtomicCounterBuffer(unsigned int Value);
	RHIBufferRef RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z);

	RHITexture2DRef RHICreateTexture2D(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, void *data
		);
	RHITexture2DArrayRef RHICreateTexture2DArray(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, void *data
		);
	RHITexture3DRef RHICreateTexture3D(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, void *data
		);
	RHITextureCubeRef RHICreateTextureCube(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, void *data[6]
		);
	RHIFramebufferRef RHICreateFramebuffer();
	RHIFramebufferRef RHICreateFramebuffer(EFramebufferAttachment attachment, RHITexture2DRef texture);
	RHIFramebufferRef RHICreateFramebuffer(
			EFramebufferAttachment attachment, RHITexture2DArrayRef texture, unsigned int layer_index
		);
	void RHIGenerateMipmap(RHITextureRef texture);

	void RHIBindComputeBuffer(uint32 index, RHIBufferRef buffer);
	void RHIBindFramebuffer(RHIFramebufferRef framebuffer);
	void RHIBindBufferData(RHIBufferRef buffer, unsigned int size, void *data, EBufferUsageFlags usage);

	void *RHIMapComputeBuffer(RHIBufferRef buffer, EDataAccessFlag access);
	void RHIUnmapComputeBuffer(RHIBufferRef buffer);
	unsigned char *RHIReadImagePixel(RHITexture2DRef texture);
	void RHISetViewport(int32 x, int32 y, int32 width, int32 height);

	void RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size);

	void RHIDrawArrays(uint32 Count, int32 InstanceCount = 1);
	void RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset);
	void RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount = 1);

	void RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z);
	void RHIDispatchIndirect(RHIBuffer *indirectArgs);
	void RHIImageMemoryBarrier();
	void RHIStorageMemoryBarrier();
	void RHIClearBuffer(uint32 flagbits);
}