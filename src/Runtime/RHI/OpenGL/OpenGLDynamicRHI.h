#pragma once
#include <array>
#include <tuple>
#include <vector>
#include <memory>

// #include "Common/CoordinateAxis.h"
// #include "Common/GfxStructures.h"
#include "OpenGL/OpenGLFramebuffer.h"
#include "OpenGL/OpenGLTexture.h"
#include "OpenGL/OpenGLState.h"
#include "RHIDefinitions.h"
#include "DynamicRHI.h"

#include "OpenGLResources.h"
#include "RHIResources.h"


namespace nilou {

	constexpr int MAX_VERTEX_ATTRIBUTE_COUNT = 32;

	class FOpenGLDynamicRHI : public FDynamicRHI
	{
	public:
		FOpenGLDynamicRHI(const GfxConfiguration& Config) : FDynamicRHI(Config) {}
		virtual ~FOpenGLDynamicRHI() {}
		virtual int Initialize() override;
		virtual void Finalize() override;
		virtual void GetError(const char *file, int line) override;
		virtual EGraphicsAPI GetCurrentGraphicsAPI() { return EGraphicsAPI::OpenGL; }

		virtual void RHIBeginFrame() {}
		virtual void RHIEndFrame() {}

		/**
		* Set state
		*/
		virtual void RHISetViewport(int32 Width, int32 Height) override;
		virtual FRHIGraphicsPipelineState *RHISetComputeShader(RHIComputeShader *ComputeShader) override;
		virtual void RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState) override;
		virtual bool RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *) override;
		virtual bool RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *) override;
		virtual bool RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI) override;
		virtual bool RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI) override;
		virtual bool RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) override;
		virtual bool RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) override;
		// virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, int32 Value) override;
		// virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, int32 Value) override;
		// virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, float Value) override;
		// virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, float Value) override;
		// virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, uint32 Value) override;
		// virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, uint32 Value) override;
		
		/**
		* Binding buffers
		*/
		virtual void RHIBindComputeBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIBuffer* buffer) override;
		virtual void RHIBindComputeBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIBuffer* buffer) override;
		virtual void RHIBindFramebuffer(RHIFramebuffer *framebuffer) override;
		virtual void RHIBindBufferData(RHIBuffer* buffer, unsigned int size, void *data) override;

		/**
		* Create/Update data
		*/
		virtual FRHIGraphicsPipelineState *RHIGetOrCreatePipelineStateObject(const FGraphicsPipelineStateInitializer &Initializer);
		virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) override;
		virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) override;
		virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) override;
		virtual RHIVertexShaderRef RHICreateVertexShader(const std::string& code) override;
		virtual RHIPixelShaderRef RHICreatePixelShader(const std::string& code) override;
		virtual RHIComputeShaderRef RHICreateComputeShader(const std::string& code) override;
		virtual void RHIDestroyShader(RHIShader* Shader) override { }
		virtual RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data) override;
		virtual RHIUniformBufferRef RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data) override;
		virtual RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data) override;
		virtual RHIBufferRef RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override;
		virtual RHIBufferRef RHICreateDrawElementsIndirectBuffer(
				int32 Count, uint32 instanceCount, uint32 firstIndex, uint32 baseVertex, uint32 baseInstance) override;
		
		virtual RHITexture2DRef RHICreateTexture2D(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY) override;
		virtual RHITexture2DArrayRef RHICreateTexture2DArray(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ) override;
		virtual RHITexture3DRef RHICreateTexture3D(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ) override;
		virtual RHITextureCubeRef RHICreateTextureCube(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY) override;
		virtual RHITexture2DRef RHICreateSparseTexture2D(
			const std::string &name, EPixelFormat Format, 
			int32 NumMips, uint32 InSizeX, uint32 InSizeY) override;

		virtual RHIFramebufferRef RHICreateFramebuffer(std::map<EFramebufferAttachment, RHITexture2DRef> Attachments) override;
		virtual void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data) override;
		virtual void RHIUpdateBuffer(RHIBuffer* Buffer, uint32 Offset, uint32 Size, void *Data) override;
		virtual RHITexture2DRef RHICreateTextureView2D(
			RHITexture* OriginTexture, EPixelFormat Format, uint32 MinLevel, uint32 NumLevels, uint32 LevelIndex
		) override;
		virtual RHITextureCubeRef RHICreateTextureViewCube(
			RHITexture* OriginTexture, EPixelFormat Format, uint32 MinMipLevel, uint32 NumMipLevels
		) override;

		virtual void RHIUpdateTexture2D(RHITexture2D* Texture, 
			int32 Xoffset, int32 Yoffset, 
			int32 Width, int32 Height, 
			int32 MipmapLevel, void* Data) override;
		virtual void RHIUpdateTexture3D(RHITexture3D* Texture, 
			int32 Xoffset, int32 Yoffset, int32 Zoffset,
			int32 Width, int32 Height, int32 Depth, 
			int32 MipmapLevel, void* Data) override;
		virtual void RHIUpdateTexture2DArray(RHITexture2DArray* Texture, 
			int32 Xoffset, int32 Yoffset, int32 LayerIndex,
			int32 Width, int32 Height,
			int32 MipmapLevel, void* Data) override;
		virtual void RHIUpdateTextureCube(RHITextureCube* Texture, 
			int32 Xoffset, int32 Yoffset, int32 LayerIndex,
			int32 Width, int32 Height,
			int32 MipmapLevel, void* Data) override;

		/**
		* Render pass
		*/
		virtual void RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo);
		virtual void RHIDrawArrays(uint32 First, uint32 Count, int32 InstanceCount = 1) override;
		virtual void RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount = 1) override;
		virtual void RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset = 0) override;
		virtual void RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override;
		virtual void RHIDispatchIndirect(RHIBuffer *indirectArgs, uint32 IndirectOffset = 0) override;
		virtual void RHIEndRenderPass() override;

		/**
		* Utils
		*/
		virtual void RHIGenerateMipmap(RHITextureRef texture) override;
		virtual void *RHILockBuffer(RHIBuffer* buffer, uint32 Offset, uint32 Size, EResourceLockMode LockMode) override;
		virtual void RHIUnlockBuffer(RHIBuffer* buffer) override;
		virtual unsigned char *RHIReadImagePixel(RHITexture2DRef texture) override;
		virtual void RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size) override;
		virtual void RHIImageMemoryBarrier() override;
		virtual void RHIStorageMemoryBarrier() override;
		virtual void RHIClearBuffer(uint32 flagbits) override;
		virtual void RHISparseTextureUnloadTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel) override;
		virtual void RHISparseTextureUpdateTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel, void* Data) override;

	private:
		void RHISetVertexBuffer(const FRHIVertexInput *);
		void RHISetRasterizerState(RHIRasterizerState *newState);
		void RHISetDepthStencilState(RHIDepthStencilState *newState, uint32 StencilRef=0);
		void RHISetBlendState(RHIBlendState *newState);
		void RHIUseShaderProgram(OpenGLLinkedProgram *program);
		OpenGLLinkedProgramRef RHICreateLinkedProgram(OpenGLVertexShader *vert, OpenGLPixelShader *pixel);
		OpenGLLinkedProgramRef RHICreateLinkedProgram(OpenGLComputeShader *comp);
		void glTexImage2D_usingTexStorage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint nummips, GLenum format, GLenum type, const void *pixels);
		void glTexImage3D_usingTexStorage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint nummips, GLenum format, GLenum type, const void *pixels);
		void EndDraw();

		class OpenGLContext
		{
		public:
			GLuint VertexArrayObject;
			class OpenGLGraphicsPipelineState *GraphicsPipelineState = nullptr;
			std::array<bool, MAX_VERTEX_ATTRIBUTE_COUNT> VertexAttributeEnabled = { false };
			OpenGLFramebuffer *Framebuffer = nullptr;
			OpenGLRasterizerState RasterizerState;
			OpenGLDepthStencilState DepthStencilState;
			OpenGLBlendState BlendState;
			uint32 StencilRef = 0;
			int32 ViewportWidth = 0;
			int32 ViewportHeight = 0;
		};
		OpenGLContext ContextState;

		class TextureUnitManager
		{
		private:
			std::vector<int> pool, indices;
			std::vector<int> allocated_units;
		public:
			const int UnitCapacity = 64;
			int Size;
			TextureUnitManager();
			int AllocUnit();
			void FreeUnit(int unit);
			void FreeAllUnit();
		};
		TextureUnitManager TexMngr;

		struct OpenGLTextureResource
		{
			GLenum Target;
			GLuint Resource;
			GLenum InternalFormat;
		};
		OpenGLTextureResource TextureResourceCast(RHITexture *texture);
	};

}