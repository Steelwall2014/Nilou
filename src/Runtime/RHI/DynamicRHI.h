#pragma once
#include <unordered_map>
#include <vector>
#include <memory>

// #include "Common/CoordinateAxis.h"
#include "GfxConfiguration.h"

#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RHITransition.h"
#include "RHICommandList.h"
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

		/**
		* Create/Update data
		*/
		virtual RHIGraphicsPipelineState *RHICreateGraphicsPipelineState(const FGraphicsPipelineStateInitializer &Initializer, const std::vector<RHIPushConstantRange>& PushConstantRanges={}) = 0;
		virtual RHIComputePipelineState *RHICreateComputePipelineState(RHIComputeShader* ComputeShader, const std::vector<RHIPushConstantRange>& PushConstantRanges={}) = 0;
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
		virtual RHIBufferRef RHICreateBuffer(const FRHIBufferCreateInfo& CreateInfo, const std::string& Name) { return RHICreateBuffer(CreateInfo.Stride, CreateInfo.Size, CreateInfo.Usage, nullptr); }
		
		// virtual RHITexture2DRef RHICreateTexture2D(
		// 	const std::string &name, EPixelFormat Format, 
		// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) = 0;
		// virtual RHITexture2DArrayRef RHICreateTexture2DArray(
		// 	const std::string &name, EPixelFormat Format, 
		// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags) = 0;
		// virtual RHITexture3DRef RHICreateTexture3D(
		// 	const std::string &name, EPixelFormat Format, 
		// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags) = 0;
		// virtual RHITextureCubeRef RHICreateTextureCube(
		// 	const std::string &name, EPixelFormat Format, 
		// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) = 0;
		// virtual RHITexture2DRef RHICreateSparseTexture2D(
		// 	const std::string &name, EPixelFormat Format, 
		// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) = 0;
		virtual RHITextureRef RHICreateTexture(const FRHITextureCreateInfo& CreateInfo, const std::string& Name) = 0;

		// virtual RHIFramebufferRef RHICreateFramebuffer(const std::array<RHITextureView*, MaxSimultaneousRenderTargets>& InAttachments, RHITextureView* InDepthStencilAttachment) = 0;
		// virtual void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data) = 0;
		// virtual void RHIUpdateBuffer(RHIBuffer* Buffer, uint32 Offset, uint32 Size, void *Data) = 0;
		virtual RHITextureViewRef RHICreateTextureView(RHITexture* Texture, const FRHITextureViewCreateInfo& CreateInfo, const std::string& Name) = 0;

		virtual FRHIVertexDeclaration* RHICreateVertexDeclaration(const FVertexDeclarationElementList& Elements) = 0;
	
		virtual void* RHIMapMemory(RHIBuffer* buffer, uint32 Offset, uint32 Size) = 0;
		virtual void RHIUnmapMemory(RHIBuffer* buffer) = 0;
		virtual uint32 RHIComputeMemorySize(RHITexture* TextureRHI) = 0;

		virtual RHIDescriptorSetLayoutRef RHICreateDescriptorSetLayout(const std::vector<RHIDescriptorSetLayoutBinding>& Bindings) = 0;
		virtual RHIDescriptorPoolRef RHICreateDescriptorPool(RHIDescriptorSetLayout* Layout, uint32 PoolSize) = 0;
		virtual RHISemaphoreRef RHICreateSemaphore() = 0;
    	virtual RHICommandList* RHICreateCommandList() = 0;
		virtual void RHISubmitCommandList(RHICommandList* RHICmdList, const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal) = 0;

	protected:
		static FDynamicRHI *DynamicRHI;
		static ivec3 SparseTextureTileSizes[(int)ETextureDimension::TextureDimensionsNum][(int)EPixelFormat::PF_MAX];
    	// std::unordered_map<uint32, RHISamplerStateRef SamplerMap;
		RHIFramebufferRef RenderToScreenFramebuffer{};
    	// void ReflectShader(RHIDescriptorSetsLayout& DescriptorSetsLayout, shaderc_compilation_result_t compile_result);
	};

	#define RHIGetError() FDynamicRHI::GetDynamicRHI()->GetError(__FILE__, __LINE__)

	// inline RHITextureRef RHICreateTexture2D(
	// 	const std::string &name, EPixelFormat Format, 
	// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags)
	// {
	// 	return FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(name, Format, NumMips, InSizeX, InSizeY, InTexCreateFlags);
	// }

	// inline RHITextureRef RHICreateTexture2DArray(
	// 	const std::string &name, EPixelFormat Format, 
	// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InArraySize, ETextureCreateFlags InTexCreateFlags)
	// {
	// 	return FDynamicRHI::GetDynamicRHI()->RHICreateTexture2DArray(name, Format, NumMips, InSizeX, InSizeY, InArraySize, InTexCreateFlags);
	// }

	// inline RHITextureRef RHICreateTexture3D(
	// 	const std::string &name, EPixelFormat Format, 
	// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags)
	// {
	// 	return FDynamicRHI::GetDynamicRHI()->RHICreateTexture3D(name, Format, NumMips, InSizeX, InSizeY, InSizeZ, InTexCreateFlags);
	// }

	// inline RHITextureRef RHICreateTextureCube(
	// 	const std::string &name, EPixelFormat Format, 
	// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags)
	// {
	// 	return FDynamicRHI::GetDynamicRHI()->RHICreateTextureCube(name, Format, NumMips, InSizeX, InSizeY, InTexCreateFlags);
	// }

	inline RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(Stride, Size, InUsage, Data);
	}

	inline RHIGraphicsPipelineState *RHICreateGraphicsPipelineState(const FGraphicsPipelineStateInitializer &Initializer, const std::vector<RHIPushConstantRange>& PushConstantRanges={})
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateGraphicsPipelineState(Initializer, PushConstantRanges);
	}

	inline RHIComputePipelineState *RHICreateComputePipelineState(RHIComputeShader* ComputeShader, const std::vector<RHIPushConstantRange>& PushConstantRanges={})
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateComputePipelineState(ComputeShader, PushConstantRanges);
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
		return FDynamicRHI::GetDynamicRHI()->RHIMapMemory(buffer, Offset, Size);
	}

	inline void RHIUnmapMemory(RHIBuffer* buffer)
	{
		FDynamicRHI::GetDynamicRHI()->RHIUnmapMemory(buffer);
	}

	inline RHIDescriptorSetLayoutRef RHICreateDescriptorSetLayout(const std::vector<RHIDescriptorSetLayoutBinding>& Bindings)
	{
		RHIDescriptorSetLayoutRef Layout = FDynamicRHI::GetDynamicRHI()->RHICreateDescriptorSetLayout(Bindings);
		// TODO: cache
		return Layout;
	}

	inline RHIDescriptorPoolRef RHICreateDescriptorPool(RHIDescriptorSetLayout* Layout, uint32 PoolSize)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateDescriptorPool(Layout, PoolSize);
	}

	inline RHITextureRef RHICreateTexture(const FRHITextureCreateInfo& CreateInfo, const std::string& Name)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateTexture(CreateInfo, Name);
	}

	inline RHITextureViewRef RHICreateTextureView(RHITexture* Texture, const FRHITextureViewCreateInfo& CreateInfo, const std::string& Name)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateTextureView(Texture, CreateInfo, Name);
	}

	inline RHIBufferRef RHICreateBuffer(const FRHIBufferCreateInfo& CreateInfo, const std::string& Name)
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(CreateInfo, Name);
	}

	inline uint32 RHIComputeMemorySize(RHITexture* TextureRHI)
	{
		return FDynamicRHI::GetDynamicRHI()->RHIComputeMemorySize(TextureRHI);
	}

	inline RHISemaphoreRef RHICreateSemaphore()
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateSemaphore();
	}

	inline RHICommandList* RHICreateCommandList()
	{
		return FDynamicRHI::GetDynamicRHI()->RHICreateCommandList();
	}

	inline void RHISubmitCommandList(RHICommandList* RHICmdList, const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal)
	{
		FDynamicRHI::GetDynamicRHI()->RHISubmitCommandList(RHICmdList, SemaphoresToWait, SemaphoresToSignal);
	}
}