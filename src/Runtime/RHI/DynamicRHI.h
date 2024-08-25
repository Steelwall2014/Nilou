#pragma once
#include <unordered_map>
#include <vector>
#include <memory>

// #include "Common/CoordinateAxis.h"
#include "GfxConfiguration.h"

#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RHI.h"

namespace glslang {
	class TProgram;
}
namespace nilou {

	enum class EGraphicsAPI
	{
		Empty,
		OpenGL,
		Vulkan,
	};

	class FDynamicRHI
	{
	public:
		static FDynamicRHI *GetDynamicRHI();
		static void CreateDynamicRHI_RenderThread(const GfxConfiguration& configs);

		FDynamicRHI(const GfxConfiguration&) {}
		virtual ~FDynamicRHI() {}
		virtual int Initialize();
		virtual void Finalize();
		virtual void GetError(const char *file, int line) = 0;
		virtual EGraphicsAPI GetCurrentGraphicsAPI() { return EGraphicsAPI::Empty; }
		static EGraphicsAPI StaticGetCurrentGraphicsAPI() { return GetDynamicRHI()->GetCurrentGraphicsAPI(); }

		virtual void RHIBeginFrame() = 0;
		virtual void RHIEndFrame() = 0;

		/**
		* Set state
		*/
		virtual void RHISetViewport(int32 Width, int32 Height) = 0;
		virtual FRHIPipelineState *RHISetComputeShader(RHIComputeShader *ComputeShader) = 0;
		virtual void RHISetGraphicsPipelineState(FRHIPipelineState *NewState) = 0;
		virtual bool RHISetShaderUniformBuffer(FRHIPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *) = 0;
		virtual bool RHISetShaderUniformBuffer(FRHIPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *) = 0;
		virtual bool RHISetShaderSampler(FRHIPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, const RHISampler &SamplerRHI) = 0;
		virtual bool RHISetShaderSampler(FRHIPipelineState *, EPipelineStage PipelineStage, int BaseIndex, const RHISampler &SamplerRHI) = 0;
		virtual bool RHISetShaderImage(FRHIPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) = 0;
		virtual bool RHISetShaderImage(FRHIPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) = 0;
		virtual void RHISetStreamSource(uint32 StreamIndex, RHIBuffer* Buffer, uint32 Offset) = 0;

		/**
		* Binding buffers
		*/
		virtual void RHIBindComputeBuffer(FRHIPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIBuffer* buffer) = 0;
		virtual void RHIBindComputeBuffer(FRHIPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIBuffer* buffer) = 0;
		virtual void RHIBindBufferData(RHIBuffer* buffer, unsigned int size, void *data) = 0;

		/**
		* Create/Update data
		*/
		virtual FRHIPipelineState *RHICreateGraphicsPipelineState(const FGraphicsPipelineStateInitializer &Initializer) = 0;
		virtual FRHIPipelineState *RHICreateComputePipelineState(RHIComputeShader* ComputeShader) = 0;
		// TODO: Delete PSO
		// virtual void RHIDeletePipelineStateObject(FRHIGraphicsPipelineState *PSO) = 0;
		virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) = 0;
		virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) = 0;
		virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) = 0;
		virtual RHISamplerStateRef RHICreateSamplerState(const FSamplerStateInitializer &Initializer) = 0;
		virtual RHIVertexShaderRef RHICreateVertexShader(const std::string& code) = 0;
		virtual RHIPixelShaderRef RHICreatePixelShader(const std::string& code) = 0;
		virtual RHIComputeShaderRef RHICreateComputeShader(const std::string& code) = 0;
		virtual RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data) = 0;
		virtual RHIUniformBufferRef RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data) = 0;
		virtual RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data) = 0;
		virtual RHIBufferRef RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) = 0;
		virtual RHIBufferRef RHICreateDrawElementsIndirectBuffer(
				int32 Count, uint32 instanceCount, uint32 firstIndex, uint32 baseVertex, uint32 baseInstance) = 0;
		
		virtual RHITexture2DRef RHICreateTexture2D(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) = 0;
		virtual RHITexture2DArrayRef RHICreateTexture2DArray(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags) = 0;
		virtual RHITexture3DRef RHICreateTexture3D(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags) = 0;
		virtual RHITextureCubeRef RHICreateTextureCube(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) = 0;
		virtual RHITexture2DRef RHICreateSparseTexture2D(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) = 0;

		virtual RHIFramebufferRef RHICreateFramebuffer(std::map<EFramebufferAttachment, RHITexture2DRef> Attachments) = 0;
		virtual void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data) = 0;
		virtual void RHIUpdateBuffer(RHIBuffer* Buffer, uint32 Offset, uint32 Size, void *Data) = 0;
		virtual RHITexture2DRef RHICreateTextureView2D(
			RHITexture* OriginTexture, EPixelFormat Format, uint32 MinMipLevel, uint32 NumMipLevels, uint32 LayerIndex
		) = 0;
		virtual RHITextureCubeRef RHICreateTextureViewCube(
			RHITexture* OriginTexture, EPixelFormat Format, uint32 MinMipLevel, uint32 NumMipLevels
		) = 0;

		virtual void RHIUpdateTexture2D(RHITexture2D* Texture, 
			int32 Xoffset, int32 Yoffset, 
			int32 Width, int32 Height, 
			int32 MipmapLevel, void* Data) = 0;
		virtual void RHIUpdateTexture3D(RHITexture3D* Texture, 
			int32 Xoffset, int32 Yoffset, int32 Zoffset,
			int32 Width, int32 Height, int32 Depth, 
			int32 MipmapLevel, void* Data) = 0;
		virtual void RHIUpdateTexture2DArray(RHITexture2DArray* Texture, 
			int32 Xoffset, int32 Yoffset, int32 LayerIndex,
			int32 Width, int32 Height,
			int32 MipmapLevel, void* Data) = 0;
		virtual void RHIUpdateTextureCube(RHITextureCube* Texture, 
			int32 Xoffset, int32 Yoffset, int32 LayerIndex,
			int32 Width, int32 Height,
			int32 MipmapLevel, void* Data) = 0;

		virtual FRHIVertexDeclaration* RHICreateVertexDeclaration(const FVertexDeclarationElementList& Elements) = 0;

		/**
		* Render pass
		*/
		virtual void RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo) = 0;
		virtual void RHIDrawArrays(uint32 First, uint32 Count, int32 InstanceCount = 1) = 0;
		virtual void RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount = 1) = 0;
		virtual void RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset = 0) = 0;
		virtual void RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) = 0;
		virtual void RHIDispatchIndirect(RHIBuffer *indirectArgs, uint32 IndirectOffset = 0) = 0;
		virtual void RHIEndRenderPass() { }

		/**
		* Utils
		*/
		virtual void RHIGenerateMipmap(RHITextureRef texture) = 0;
		virtual void *RHILockBuffer(RHIBuffer* buffer, uint32 Offset, uint32 Size, EResourceLockMode LockMode) = 0;
		virtual void RHIUnlockBuffer(RHIBuffer* buffer) = 0;
		virtual unsigned char *RHIReadImagePixel(RHITexture2DRef texture) = 0;
		virtual void RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size) = 0;
		virtual void RHIImageMemoryBarrier() = 0;
		virtual void RHIStorageMemoryBarrier() = 0;
		virtual void RHIClearBuffer(uint32 flagbits) = 0;
		virtual void RHISparseTextureUnloadTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel) = 0;
		virtual void RHISparseTextureUpdateTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel, void* Data) = 0;
		
		// The page size is represented as X, Y and Z.
		// They correspond to column, row, and channel.
		// The direction of each axis is the same as UV's.
		static ivec3 RHIGetSparseTexturePageSize(ETextureDimension TextureType, EPixelFormat PixelFormat);

		virtual RHIFramebuffer* GetRenderToScreenFramebuffer() { return RenderToScreenFramebuffer.get(); }
	
		virtual void* MapMemory(RHIBuffer* buffer, uint32 Offset, uint32 Size) = 0;
		virtual void UnmapMemory(RHIBuffer* buffer) = 0;
		virtual RHIDescriptorSetLayoutRef CreateDescriptorSetLayout(const std::vector<RHIDescriptorSetLayoutBinding>& Bindings) = 0;
		virtual RHIDescriptorPoolRef CreateDescriptorPool(RHIDescriptorSetLayout* Layout, uint32 PoolSize) = 0;
		virtual RHIDescriptorSet* AllocateDescriptorSet(RHIDescriptorPool* Pool) = 0;

	protected:
		static FDynamicRHI *DynamicRHI;
		static ivec3 SparseTextureTileSizes[(int)ETextureDimension::TextureDimensionsNum][(int)EPixelFormat::PF_MAX];
    	// std::unordered_map<uint32, RHISamplerStateRef> SamplerMap;
		RHIFramebufferRef RenderToScreenFramebuffer{};
    	// void ReflectShader(RHIDescriptorSetsLayout& DescriptorSetsLayout, shaderc_compilation_result_t compile_result);
	};

	#define RHIGetError() FDynamicRHI::GetDynamicRHI()->GetError(__FILE__, __LINE__)

	inline RHITextureRef RHICreateTexture2D(
		const std::string &name, EPixelFormat Format, 
		int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(name, Format, NumMips, InSizeX, InSizeY, InTexCreateFlags);
	}

	inline RHITextureRef RHICreateTexture2DArray(
		const std::string &name, EPixelFormat Format, 
		int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InArraySize, ETextureCreateFlags InTexCreateFlags)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateTexture2DArray(name, Format, NumMips, InSizeX, InSizeY, InArraySize, InTexCreateFlags);
	}

	inline RHITextureRef RHICreateTexture3D(
		const std::string &name, EPixelFormat Format, 
		int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateTexture3D(name, Format, NumMips, InSizeX, InSizeY, InSizeZ, InTexCreateFlags);
	}

	inline RHITextureRef RHICreateTextureCube(
		const std::string &name, EPixelFormat Format, 
		int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateTextureCube(name, Format, NumMips, InSizeX, InSizeY, InTexCreateFlags);
	}

	inline RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(Stride, Size, InUsage, Data);
	}

	inline FRHIPipelineState *RHICreateGraphicsPipelineState(const FGraphicsPipelineStateInitializer &Initializer)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateGraphicsPipelineState(Initializer);
	}

	inline FRHIPipelineState *RHICreateComputePipelineState(RHIComputeShader* ComputeShader)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateComputePipelineState(ComputeShader);
	}

	inline FRHIVertexDeclaration* RHICreateVertexDeclaration(const FVertexDeclarationElementList& ElementList)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateVertexDeclaration(ElementList);
	}

	inline RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer& Initializer)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateDepthStencilState(Initializer);
	}

	inline RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer& Initializer)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateRasterizerState(Initializer);
	}

	inline RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer& Initializer)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateBlendState(Initializer);
	}

	inline RHISamplerStateRef RHICreateSamplerState(const FSamplerStateInitializer& Initializer)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateSamplerState(Initializer);
	}
	
	inline RHIVertexShaderRef RHICreateVertexShader(const std::string& code)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateVertexShader(code);
	}
	
	inline RHIPixelShaderRef RHICreatePixelShader(const std::string& code)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreatePixelShader(code);
	}
	
	inline RHIComputeShaderRef RHICreateComputeShader(const std::string& code)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateComputeShader(code);
	}

	inline EGraphicsAPI RHIGetCurrentGraphicsAPI()
	{
		return FDynamicRHI::GetDynamicRHI()->GetCurrentGraphicsAPI();
	}

	inline void* RHIMapMemory(RHIBuffer* buffer, uint32 Offset, uint32 Size)
	{
		return FDynamicRHI::GetDynamicRHI()->MapMemory(buffer, Offset, Size);
	}

	inline void RHIUnmapMemory(RHIBuffer* buffer)
	{
		FDynamicRHI::GetDynamicRHI()->UnmapMemory(buffer);
	}

	inline RHIDescriptorSetLayout* RHICreateDescriptorSetLayout(const std::vector<RHIDescriptorSetLayoutBinding>& Bindings)
	{
		RHIDescriptorSetLayoutRef Layout = FDynamicRHI::GetDynamicRHI()->CreateDescriptorSetLayout(Bindings);
		// TODO: cache
	}

	inline RHIDescriptorPoolRef RHICreateDescriptorPool(RHIDescriptorSetLayout* Layout, uint32 PoolSize)
	{
		return FDynamicRHI::GetDynamicRHI()->CreateDescriptorPool(Layout, PoolSize);
	}

	inline RHIDescriptorSet* RHIAllocateDescriptorSet(RHIDescriptorPool* Pool)
	{
		return FDynamicRHI::GetDynamicRHI()->AllocateDescriptorSet(Pool);
	}
}