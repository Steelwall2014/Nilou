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
		FOpenGLDynamicRHI() {}
		virtual ~FOpenGLDynamicRHI() {}
		virtual int Initialize() override;
		virtual void Finalize() override;
		virtual void GetError(const char *file, int line) override;
		virtual EGraphicsAPI GetCurrentGraphicsAPI() { return EGraphicsAPI::OpenGL; }

		/**
		* Set state
		*/
		virtual void RHISetViewport(int32 Width, int32 Height) override;
		virtual FRHIGraphicsPipelineState *RHISetComputeShader(FShaderInstance *ComputeShader) override;
		virtual void RHISetBlendState(RHIBlendState *newState) override;
		virtual void RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState) override;
		virtual bool RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *) override;
		virtual bool RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *) override;
		virtual bool RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI) override;
		virtual bool RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI) override;
		virtual bool RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) override;
		virtual bool RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) override;
		virtual void RHISetVertexBuffer(FRHIGraphicsPipelineState *, FRHIVertexInput *) override;
		virtual void RHISetRasterizerState(RHIRasterizerState *newState) override;
		virtual void RHISetDepthStencilState(RHIDepthStencilState *newState, uint32 StencilRef=0) override;

		/**
		* Create/Update data
		*/
		virtual FRHIGraphicsPipelineState *RHIGetOrCreatePipelineStateObject(const FRHIGraphicsPipelineInitializer &StateData);
		virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) override;
		virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) override;
		virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) override;
		virtual RHIVertexShaderRef RHICreateVertexShader(const char *code) override;
		virtual RHIPixelShaderRef RHICreatePixelShader(const char *code) override;
		virtual RHIComputeShaderRef RHICreateComputeShader(const char *code) override;
		virtual RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data) override;
		virtual RHIUniformBufferRef RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data) override;
		virtual RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data) override;
		virtual RHIBufferRef RHICreateAtomicCounterBuffer(unsigned int Value) override;
		virtual RHIBufferRef RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override;
		virtual RHITexture2DRef RHICreateTexture2D(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, void *data
		) override;
		virtual RHITexture2DArrayRef RHICreateTexture2DArray(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, void *data
		) override;
		virtual RHITexture3DRef RHICreateTexture3D(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, void *data
		) override;
		virtual RHITextureCubeRef RHICreateTextureCube(
			const std::string &name, EPixelFormat Format, int32 NumMips, uint32 InSizeX, uint32 InSizeY, void *data[6]
		) override;
		virtual RHIFramebufferRef RHICreateFramebuffer() override;
		virtual RHIFramebufferRef RHICreateFramebuffer(EFramebufferAttachment attachment, RHITexture2DRef texture) override;
		virtual RHIFramebufferRef RHICreateFramebuffer(
			EFramebufferAttachment attachment, RHITexture2DArrayRef texture, unsigned int layer_index
		) override;
		virtual void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data) override;
		
		/**
		* Binding buffers
		*/
		virtual void RHIBindComputeBuffer(uint32 index, RHIBufferRef buffer) override;
		virtual void RHIBindFramebuffer(RHIFramebuffer *framebuffer) override;
		virtual void RHIBindBufferData(RHIBufferRef buffer, unsigned int size, void *data, EBufferUsageFlags usage) override;

		/**
		* Render pass
		*/
		virtual void RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo);
		virtual void RHIDrawArrays(uint32 First, uint32 Count, int32 InstanceCount = 1) override;
		virtual void RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount = 1) override;
		virtual void RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset = 0) override;
		virtual void RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override;
		virtual void RHIDispatchIndirect(RHIBuffer *indirectArgs) override;
		virtual void RHIEndRenderPass() override;

		/**
		* Utils
		*/
		virtual FRHIRenderQueryRef RHICreateRenderQuery() override;
		virtual void RHIBeginRenderQuery(FRHIRenderQuery* RenderQuery) override;
		virtual void RHIEndRenderQuery(FRHIRenderQuery* RenderQuery) override;
		virtual void RHIGetRenderQueryResult(FRHIRenderQuery* RenderQuery) override;
		virtual void RHIGenerateMipmap(RHITextureRef texture) override;
		virtual void *RHIMapComputeBuffer(RHIBufferRef buffer, EDataAccessFlag access) override;
		virtual void RHIUnmapComputeBuffer(RHIBufferRef buffer) override;
		virtual unsigned char *RHIReadImagePixel(RHITexture2DRef texture) override;
		virtual void RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size) override;
		virtual void RHIImageMemoryBarrier() override;
		virtual void RHIStorageMemoryBarrier() override;
		virtual void RHIClearBuffer(uint32 flagbits) override;

	private:
		OpenGLLinkedProgramRef m_CurrentShader;
		void RHIUseShaderProgram(OpenGLLinkedProgram *program);
		OpenGLLinkedProgramRef RHICreateLinkedProgram(OpenGLVertexShader *vert, OpenGLPixelShader *pixel);
		OpenGLLinkedProgramRef RHICreateLinkedProgram(OpenGLComputeShader *comp);
		void glTexImage2D_usingTexStorage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
		void glTexImage3D_usingTexStorage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
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