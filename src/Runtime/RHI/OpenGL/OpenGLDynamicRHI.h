#pragma once
#include <array>
#include <tuple>
#include <vector>
#include <memory>

#include "Interface/IRuntimeModule.h"
#include "Interface/IDrawPass.h"
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

	// class FOpenGLRHIModule : public IDynamicRHIModule
	// {
	// public:
	// 	FOpenGLRHIModule();
	// 	virtual int StartupModule() override;		
	// 	virtual void ShutdownModule() override;
	// 	virtual class FDynamicRHI* CreateRHI() override;
	// };

	class FOpenGLDynamicRHI : public FDynamicRHI // : public GraphicsManager
	{
	public:
		FOpenGLDynamicRHI() {}
		// virtual bool RHISetShaderParameter(const char *paramName, const glm::mat4 &param) override;
		// virtual bool RHISetShaderParameter(const char *paramName, const glm::vec2 &param) override;
		// virtual bool RHISetShaderParameter(const char *paramName, const glm::vec3 &param) override;
		// virtual bool RHISetShaderParameter(const char *paramName, const glm::vec4 &param) override;
		// virtual bool RHISetShaderParameter(const char *paramName, const float param) override;
		// virtual bool RHISetShaderParameter(const char *paramName, const int32_t param) override;
		// virtual bool RHISetShaderParameter(const char *paramName, const uint32_t param) override;
		// virtual bool RHISetShaderParameter(const char *paramName, const bool param) override;

		virtual void RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int32 BaseIndex, RHIUniformBuffer *) override;
		virtual void RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int32 BaseIndex, const FRHISampler &SamplerRHI) override;
		virtual void RHISetShaderImage(FRHIGraphicsPipelineState *, int32 BaseIndex, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) override;
		virtual void RHISetVertexBuffer(FRHIGraphicsPipelineState *, FRHIVertexInput *) override;

	public:

		virtual EGraphicsAPI GetCurrentGraphicsAPI() { return EGraphicsAPI::OpenGL; }

		virtual void RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo);

		virtual FRHIGraphicsPipelineState *RHIGetBoundPipelineState() override;

		virtual FRHIGraphicsPipelineState *RHIGetOrCreatePipelineStateObject(const FRHIGraphicsPipelineInitializer &StateData);

		// virtual FRHIVertexDeclarationRef RHICreateVertexDeclaration(const FVertexDeclarationElementList &ElementList);
		
		virtual void RHISetRasterizerState(RHIRasterizerState *newState) override;
		virtual void RHISetDepthStencilState(RHIDepthStencilState *newState, uint32 StencilRef=0) override;
		virtual void RHISetBlendState(RHIBlendState *newState) override;
		virtual void RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState) override;

		virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) override;
		virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) override;
		virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) override;
		// virtual FRHIGraphicsPipelineStateRef RHICreateGraphicsPipelineState(const FRHIGraphicsPipelineInitializer &NewState) override;

		virtual RHIVertexShaderRef RHICreateVertexShader(const char *code) override;
		virtual RHIPixelShaderRef RHICreatePixelShader(const char *code) override;
		virtual RHIComputeShaderRef RHICreateComputeShader(const char *code) override;

		// virtual RHIVertexArrayObjectRef RHICreateVertexArrayObject(EPrimitiveMode Mode) override;
		virtual RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data) override;
		virtual RHIUniformBufferRef RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data) override;
		// virtual RHIBufferRef RHICreateIndexArrayBuffer(int DataByteSize, void *Data, GLenum DataType, int Count, GLenum Usage = GL_STATIC_DRAW) override;
		
		virtual void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data) override;

		// virtual std::pair<RHIShaderResourceView, RHIBufferRef> RHICreateVertexAttribBuffer(int32 AttribIndex, int DataByteSize, void *Data, EVertexElementTypeFlags Type, EBufferUsageFlags Usage) override;
		virtual RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data) override;
		virtual RHIBufferRef RHICreateAtomicCounterBuffer(unsigned int Value) override;
		// virtual RHIBufferRef RHICreateDrawArraysIndirectBuffer(RHIVertexArrayObjectRef VAO) override;
		// virtual RHIBufferRef RHICreateDrawElementsIndirectBuffer(RHIVertexArrayObjectRef VAO) override;
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
		// virtual RHITextureImage2DRef RHICreateTextureImage2D(
		// 	std::string name, unsigned int PixelFormat, unsigned int width, unsigned int height, bool mipmap=false
		// );
		// virtual RHITextureImage3DRef RHICreateTextureImage3D(
		// 	std::string name, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int depth, bool mipmap = false
		// );
		virtual RHIFramebufferRef RHICreateFramebuffer() override;
		virtual RHIFramebufferRef RHICreateFramebuffer(EFramebufferAttachment attachment, RHITexture2DRef texture) override;
		virtual RHIFramebufferRef RHICreateFramebuffer(
			EFramebufferAttachment attachment, RHITexture2DArrayRef texture, unsigned int layer_index
		) override;
		virtual void RHIGenerateMipmap(RHITextureRef texture) override;

		// virtual void RHIBindVertexArrayObject(RHIVertexArrayObjectRef VAO) override;
		virtual void RHIBindComputeBuffer(uint32 index, RHIBufferRef buffer) override;
		virtual void RHIBindFramebuffer(RHIFramebuffer *framebuffer) override;
		// virtual void RHIBindTexture(const std::string &shaderParamName, RHITextureRef texture, const RHITextureParams &params = RHITextureParams::DefaultParams) override;
		// virtual void RHIBindTexture(uint32 Unit, RHITextureRef texture, EDataAccessFlag Access) override;
		virtual void RHIBindBufferData(RHIBufferRef buffer, unsigned int size, void *data, EBufferUsageFlags usage) override;

		// virtual void RHIInitializeVertexArrayObject(RHIVertexArrayObjectRef VAO) override;
		virtual void *RHIMapComputeBuffer(RHIBufferRef buffer, EDataAccessFlag access) override;
		virtual void RHIUnmapComputeBuffer(RHIBufferRef buffer) override;
		virtual unsigned char *RHIReadImagePixel(RHITexture2DRef texture) override;
		virtual void RHISetViewport(int32 x, int32 y, int32 width, int32 height) override;

		virtual void RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size) override;

		virtual void RHIDrawArrays(uint32 Count, int32 InstanceCount = 1) override;
		virtual void RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount = 1) override;
		virtual void RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset = 0) override;

		// virtual void RHIDrawVertexArray(RHIVertexArrayObjectRef VAO, RHIFramebufferRef RenderTarget=nullptr) override;
		// virtual void RHIDrawVertexArrayInstanced(RHIVertexArrayObjectRef VAO, unsigned int instancecount, RHIFramebufferRef RenderTarget = nullptr) override;
		// virtual void RHIDrawVertexArrayIndirect(RHIVertexArrayObjectRef VAO, RHIBufferRef indirectArgs, RHIFramebufferRef RenderTarget = nullptr) override;
		virtual void RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override;
		virtual void RHIDispatchIndirect(RHIBuffer *indirectArgs) override;
		virtual void RHIImageMemoryBarrier() override;
		virtual void RHIStorageMemoryBarrier() override;
		virtual void RHIClearBuffer(uint32 flagbits) override;

		virtual ~FOpenGLDynamicRHI() {}
		virtual int Initialize() override;
		virtual void Finalize() override;
		// virtual void Tick(double DeltaTime) override;
		// virtual void Clear() override;
		// virtual void Draw() override;

		virtual void GLDEBUG();

	private:
		// CoordinateAxis axis;
		OpenGLLinkedProgramRef m_CurrentShader;
		// std::vector<int> m_AllocatedUnitSinceLastDrawcall;
		// void InitializeDefaultMaterial();
		// void InitializeDrawPasses();
		void RHIUseShaderProgram(OpenGLLinkedProgram *program);
		OpenGLLinkedProgramRef RHICreateLinkedProgram(OpenGLVertexShader *vert, OpenGLPixelShader *pixel);
		OpenGLLinkedProgramRef RHICreateLinkedProgram(OpenGLComputeShader *comp);
		// RHIClearTextureUnit();
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
			uint32 StencilRef;
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

	extern FDynamicRHI *GDynamicRHI;

}