#pragma once
#include <unordered_map>
#include <vector>
#include <memory>

// #include "Common/CoordinateAxis.h"
#include "GfxConfiguration.h"

#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RHI.h"


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
		virtual FRHIGraphicsPipelineState *RHISetComputeShader(RHIComputeShader *ComputeShader) = 0;
		virtual void RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState) = 0;
		virtual bool RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *) = 0;
		virtual bool RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *) = 0;
		virtual bool RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI) = 0;
		virtual bool RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI) = 0;
		virtual bool RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) = 0;
		virtual bool RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) = 0;
		virtual void RHISetStreamSource(uint32 StreamIndex, RHIBuffer* Buffer, uint32 Offset) = 0;

		/**
		* Binding buffers
		*/
		virtual void RHIBindComputeBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIBuffer* buffer) = 0;
		virtual void RHIBindComputeBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIBuffer* buffer) = 0;
		virtual void RHIBindBufferData(RHIBuffer* buffer, unsigned int size, void *data) = 0;

		/**
		* Create/Update data
		*/
		virtual FRHIGraphicsPipelineState *RHIGetOrCreatePipelineStateObject(const FGraphicsPipelineStateInitializer &Initializer) = 0;
		virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) = 0;
		virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) = 0;
		virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) = 0;
		virtual RHIVertexShaderRef RHICreateVertexShader(const std::string& code) = 0;
		virtual RHIPixelShaderRef RHICreatePixelShader(const std::string& code) = 0;
		virtual RHIComputeShaderRef RHICreateComputeShader(const std::string& code) = 0;
		virtual void RHIDestroyShader(RHIShader* Shader) = 0;
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
		virtual RHISamplerStateRef RHICreateSamplerState(const RHITextureParams& Params) = 0;

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

		virtual FRHIVertexDeclarationRef RHICreateVertexDeclaration(const std::vector<FVertexElement>& Elements) = 0;

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
		static ivec3 RHIGetSparseTexturePageSize(ETextureType TextureType, EPixelFormat PixelFormat);

		virtual RHIFramebuffer* GetRenderToScreenFramebuffer() { return RenderToScreenFramebuffer.get(); }
	protected:
		void AllocateParameterBindingPoint(FRHIPipelineLayout* PipelineLayout, const FGraphicsPipelineStateInitializer &Initializer);
		static FDynamicRHI *DynamicRHI;
		static ivec3 SparseTextureTileSizes[(int)ETextureType::TT_TextureTypeNum][(int)EPixelFormat::PF_PixelFormatNum];
    	std::unordered_map<uint32, RHISamplerStateRef> SamplerMap;
		RHIFramebufferRef RenderToScreenFramebuffer{};
	};

	#define RHIGetError() FDynamicRHI::GetDynamicRHI()->GetError(__FILE__, __LINE__)
}