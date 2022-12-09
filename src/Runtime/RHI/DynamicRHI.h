#pragma once
#include <unordered_map>
#include <vector>
#include <memory>

#include "Interface/IRuntimeModule.h"
#include "Interface/IDrawPass.h"
// #include "Common/CoordinateAxis.h"
//#include "Common/GfxStructures.h"

#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RHI.h"
#include "VertexFactory.h"
#include "Modules/ModuleManager.h"


namespace nilou {

	class IDynamicRHIModule : public IRuntimeModule
	{
	public:
		/** Creates a new instance of the dynamic RHI implemented by the module. */
		virtual class FDynamicRHI* CreateRHI() = 0;
	};

	enum class EGraphicsAPI
	{
		Empty,
		OpenGL,
		Vulkan,
	};

	class FDynamicRHI //: implements IRuntimeModule // : public GraphicsManager
	{
	public:
		FDynamicRHI() {}
		virtual ~FDynamicRHI() {}
		virtual int Initialize() = 0;
		virtual void Finalize() = 0;
		virtual void GetError(const char *file, int line) = 0;
		virtual EGraphicsAPI GetCurrentGraphicsAPI() { return EGraphicsAPI::Empty; }

		/**
		* Set state
		*/
		virtual void RHISetViewport(int32 x, int32 y, int32 width, int32 height) = 0;
		virtual FRHIGraphicsPipelineState *RHISetComputeShader(FShaderInstance *ComputeShader) = 0;
		virtual void RHISetBlendState(RHIBlendState *newState) = 0;
		virtual void RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState) = 0;
		virtual void RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *) = 0;
		virtual void RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *) = 0;
		virtual void RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI) = 0;
		virtual void RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI) = 0;
		virtual void RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) = 0;
		virtual void RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) = 0;
		virtual void RHISetVertexBuffer(FRHIGraphicsPipelineState *, FRHIVertexInput *) = 0;
		virtual void RHISetRasterizerState(RHIRasterizerState *newState) = 0;
		virtual void RHISetDepthStencilState(RHIDepthStencilState *newState, uint32 StencilRef=0) = 0;

		/**
		* Create/Update data
		*/
		virtual FRHIGraphicsPipelineState *RHIGetOrCreatePipelineStateObject(const FRHIGraphicsPipelineInitializer &StateData) = 0;
		virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) = 0;
		virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) = 0;
		virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) = 0;
		virtual RHIVertexShaderRef RHICreateVertexShader(const char *code) = 0;
		virtual RHIPixelShaderRef RHICreatePixelShader(const char *code) = 0;
		virtual RHIComputeShaderRef RHICreateComputeShader(const char *code) = 0;
		virtual RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data) = 0;
		virtual RHIUniformBufferRef RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data) = 0;
		virtual RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data) = 0;
		virtual RHIBufferRef RHICreateAtomicCounterBuffer(unsigned int Value) = 0;
		virtual RHIBufferRef RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) = 0;
		virtual RHITexture2DRef RHICreateTexture2D(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, void *data
		) = 0;
		virtual RHITexture2DArrayRef RHICreateTexture2DArray(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, void *data
		) = 0;
		virtual RHITexture3DRef RHICreateTexture3D(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, void *data
		) = 0;
		virtual RHITextureCubeRef RHICreateTextureCube(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, void *data[6]
		) = 0;
		virtual RHIFramebufferRef RHICreateFramebuffer() = 0;
		virtual RHIFramebufferRef RHICreateFramebuffer(EFramebufferAttachment attachment, RHITexture2DRef texture) = 0;
		virtual RHIFramebufferRef RHICreateFramebuffer(
			EFramebufferAttachment attachment, RHITexture2DArrayRef texture, unsigned int layer_index
		) = 0;
		virtual void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data) = 0;
		
		/**
		* Binding buffers
		*/
		virtual void RHIBindComputeBuffer(uint32 index, RHIBufferRef buffer) = 0;
		virtual void RHIBindFramebuffer(RHIFramebuffer *framebuffer) = 0;
		virtual void RHIBindBufferData(RHIBufferRef buffer, unsigned int size, void *data, EBufferUsageFlags usage) = 0;

		/**
		* Render pass
		*/
		virtual void RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo) = 0;
		virtual void RHIDrawArrays(uint32 Count, int32 InstanceCount = 1) = 0;
		virtual void RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount = 1) = 0;
		virtual void RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset = 0) = 0;
		virtual void RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) = 0;
		virtual void RHIDispatchIndirect(RHIBuffer *indirectArgs) = 0;
		virtual void RHIEndRenderPass() { }

		/**
		* Utils
		*/
		virtual void RHIGenerateMipmap(RHITextureRef texture) = 0;
		virtual void *RHIMapComputeBuffer(RHIBufferRef buffer, EDataAccessFlag access) = 0;
		virtual void RHIUnmapComputeBuffer(RHIBufferRef buffer) = 0;
		virtual unsigned char *RHIReadImagePixel(RHITexture2DRef texture) = 0;
		virtual void RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size) = 0;
		virtual void RHIImageMemoryBarrier() = 0;
		virtual void RHIStorageMemoryBarrier() = 0;
		virtual void RHIClearBuffer(uint32 flagbits) = 0;

	protected:
		std::map<FRHIGraphicsPipelineInitializer, FRHIGraphicsPipelineStateRef> CachedPipelineStateObjects;
	};

	extern FDynamicRHI *GDynamicRHI;

	#define RHIGetError() GDynamicRHI->GetError(__FILE__, __LINE__)
}