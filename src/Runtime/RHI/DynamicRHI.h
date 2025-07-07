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
		static FDynamicRHI *Get();
		static void CreateDynamicRHI_RenderThread(const GfxConfiguration& configs);

		FDynamicRHI(const GfxConfiguration&) {}
		virtual ~FDynamicRHI() {}
		virtual int Initialize();
		virtual void Finalize();
		virtual void GetError(const char *file, int line) = 0;
		virtual EGraphicsAPI GetCurrentGraphicsAPI() { return EGraphicsAPI::Empty; }
		static EGraphicsAPI StaticGetCurrentGraphicsAPI() { return Get()->GetCurrentGraphicsAPI(); }

		virtual void RHIBeginFrame() = 0;
		virtual void RHIEndFrame() = 0;
		virtual RHITexture* RHIGetSwapChainTexture() = 0;
		/**
		* Create/Update data
		*/
		virtual RHIGraphicsPipelineState *RHICreateGraphicsPipelineState(const FGraphicsPipelineStateInitializer &Initializer) = 0;
		virtual RHIComputePipelineState *RHICreateComputePipelineState(RHIComputeShader* ComputeShader) = 0;
		// TODO: Delete PSO
		// virtual void RHIDeletePipelineStateObject(FRHIGraphicsPipelineState *PSO) = 0;
		virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) = 0;
		virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) = 0;
		virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) = 0;
		virtual RHISamplerStateRef RHICreateSamplerState(const FSamplerStateInitializer &Initializer) = 0;
		virtual RHIVertexShaderRef RHICreateVertexShader(const std::string& code, const std::string& DebugName) = 0;
		virtual RHIPixelShaderRef RHICreatePixelShader(const std::string& code, const std::string& DebugName) = 0;
		virtual RHIComputeShaderRef RHICreateComputeShader(const std::string& code, const std::string& DebugName) = 0;
		virtual RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, const void *Data) = 0;
		virtual RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data) = 0;
		virtual RHIBufferRef RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) = 0;
		virtual RHIBufferRef RHICreateDrawElementsIndirectBuffer(
				int32 Count, uint32 instanceCount, uint32 firstIndex, uint32 baseVertex, uint32 baseInstance) = 0;
		virtual RHIBufferRef RHICreateBuffer(const FRHIBufferCreateInfo& CreateInfo, const std::string& Name) { return RHICreateBuffer(CreateInfo.Stride, CreateInfo.Size, CreateInfo.Usage, nullptr); }
		virtual RHIBuffer* RHICreateStagingBuffer(uint32 Size) = 0;
		
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
    	virtual RHICommandList* RHICreateGfxCommandList() = 0;
    	virtual RHICommandList* RHICreateComputeCommandList() = 0;
    	virtual RHICommandList* RHICreateTransferCommandList() = 0;
		virtual void RHISubmitCommandList(RHICommandList* RHICmdList, const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal) = 0;

	protected:
		static FDynamicRHI *DynamicRHI;
		static ivec3 SparseTextureTileSizes[(int)ETextureDimension::TextureDimensionsNum][(int)EPixelFormat::PF_MAX];
    	// std::unordered_map<uint32, RHISamplerStateRef SamplerMap;
		RHIFramebufferRef RenderToScreenFramebuffer{};
    	// void ReflectShader(RHIDescriptorSetsLayout& DescriptorSetsLayout, shaderc_compilation_result_t compile_result);
	};

	#define RHIGetError() FDynamicRHI::Get()->GetError(__FILE__, __LINE__)

	// inline RHITextureRef RHICreateTexture2D(
	// 	const std::string &name, EPixelFormat Format, 
	// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags)
	// {
	// 	return FDynamicRHI::Get()->RHICreateTexture2D(name, Format, NumMips, InSizeX, InSizeY, InTexCreateFlags);
	// }

	// inline RHITextureRef RHICreateTexture2DArray(
	// 	const std::string &name, EPixelFormat Format, 
	// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InArraySize, ETextureCreateFlags InTexCreateFlags)
	// {
	// 	return FDynamicRHI::Get()->RHICreateTexture2DArray(name, Format, NumMips, InSizeX, InSizeY, InArraySize, InTexCreateFlags);
	// }

	// inline RHITextureRef RHICreateTexture3D(
	// 	const std::string &name, EPixelFormat Format, 
	// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags)
	// {
	// 	return FDynamicRHI::Get()->RHICreateTexture3D(name, Format, NumMips, InSizeX, InSizeY, InSizeZ, InTexCreateFlags);
	// }

	// inline RHITextureRef RHICreateTextureCube(
	// 	const std::string &name, EPixelFormat Format, 
	// 	int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags)
	// {
	// 	return FDynamicRHI::Get()->RHICreateTextureCube(name, Format, NumMips, InSizeX, InSizeY, InTexCreateFlags);
	// }

	inline void RHIBeginFrame()
	{
		FDynamicRHI::Get()->RHIBeginFrame();
	}

	inline void RHIEndFrame()
	{
		FDynamicRHI::Get()->RHIEndFrame();
	}	

	inline RHITexture* RHIGetSwapChainTexture()
	{
		return FDynamicRHI::Get()->RHIGetSwapChainTexture();
	}

	inline RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, const void *Data)
	{
		return FDynamicRHI::Get()->RHICreateBuffer(Stride, Size, InUsage, Data);
	}

	inline RHIBuffer* RHICreateStagingBuffer(uint32 Size)
	{
		return FDynamicRHI::Get()->RHICreateStagingBuffer(Size);
	}

	inline RHIGraphicsPipelineState *RHICreateGraphicsPipelineState(const FGraphicsPipelineStateInitializer &Initializer)
	{
		return FDynamicRHI::Get()->RHICreateGraphicsPipelineState(Initializer);
	}

	inline RHIComputePipelineState *RHICreateComputePipelineState(RHIComputeShader* ComputeShader)
	{
		return FDynamicRHI::Get()->RHICreateComputePipelineState(ComputeShader);
	}

	inline FRHIVertexDeclaration* RHICreateVertexDeclaration(const FVertexDeclarationElementList& ElementList)
	{
		return FDynamicRHI::Get()->RHICreateVertexDeclaration(ElementList);
	}

	inline RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer& Initializer)
	{
		return FDynamicRHI::Get()->RHICreateDepthStencilState(Initializer);
	}

	inline RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer& Initializer)
	{
		return FDynamicRHI::Get()->RHICreateRasterizerState(Initializer);
	}

	inline RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer& Initializer)
	{
		return FDynamicRHI::Get()->RHICreateBlendState(Initializer);
	}

	inline RHISamplerStateRef RHICreateSamplerState(const FSamplerStateInitializer& Initializer)
	{
		return FDynamicRHI::Get()->RHICreateSamplerState(Initializer);
	}
	
	inline RHIVertexShaderRef RHICreateVertexShader(const std::string& code, const std::string& DebugName)
	{
		return FDynamicRHI::Get()->RHICreateVertexShader(code, DebugName);
	}
	
	inline RHIPixelShaderRef RHICreatePixelShader(const std::string& code, const std::string& DebugName)
	{
		return FDynamicRHI::Get()->RHICreatePixelShader(code, DebugName);
	}
	
	inline RHIComputeShaderRef RHICreateComputeShader(const std::string& code, const std::string& DebugName)
	{
		return FDynamicRHI::Get()->RHICreateComputeShader(code, DebugName);
	}

	inline EGraphicsAPI RHIGetCurrentGraphicsAPI()
	{
		return FDynamicRHI::Get()->GetCurrentGraphicsAPI();
	}

	inline void* RHIMapMemory(RHIBuffer* buffer, uint32 Offset, uint32 Size)
	{
		return FDynamicRHI::Get()->RHIMapMemory(buffer, Offset, Size);
	}

	inline void RHIUnmapMemory(RHIBuffer* buffer)
	{
		FDynamicRHI::Get()->RHIUnmapMemory(buffer);
	}

	inline RHIDescriptorSetLayoutRef RHICreateDescriptorSetLayout(std::vector<RHIDescriptorSetLayoutBinding> Bindings)
	{
		std::sort(Bindings.begin(), Bindings.end(), [](const RHIDescriptorSetLayoutBinding& a, const RHIDescriptorSetLayoutBinding& b) {
			return a.BindingIndex < b.BindingIndex;
		});
		for (int i = 0; i < Bindings.size()-1; i++)
		{
			if (Bindings[i].BindingIndex == Bindings[i+1].BindingIndex)
			{
				NILOU_LOG(Error, "Descriptor set layout binding index is not unique");
				NILOU_LOG(Error, "Binding index: {}, Name: {}", Bindings[i].BindingIndex, Bindings[i].Name);
				NILOU_LOG(Error, "Binding index: {}, Name: {}", Bindings[i+1].BindingIndex, Bindings[i+1].Name);
				return nullptr;
			}
		}
		return FDynamicRHI::Get()->RHICreateDescriptorSetLayout(Bindings);
	}

	inline RHIDescriptorPoolRef RHICreateDescriptorPool(RHIDescriptorSetLayout* Layout, uint32 PoolSize)
	{
		return FDynamicRHI::Get()->RHICreateDescriptorPool(Layout, PoolSize);
	}

	inline RHITextureRef RHICreateTexture(const FRHITextureCreateInfo& CreateInfo, const std::string& Name)
	{
		return FDynamicRHI::Get()->RHICreateTexture(CreateInfo, Name);
	}

	inline RHITextureViewRef RHICreateTextureView(RHITexture* Texture, const FRHITextureViewCreateInfo& CreateInfo, const std::string& Name)
	{
		return FDynamicRHI::Get()->RHICreateTextureView(Texture, CreateInfo, Name);
	}

	inline RHIBufferRef RHICreateBuffer(const FRHIBufferCreateInfo& CreateInfo, const std::string& Name)
	{
		return FDynamicRHI::Get()->RHICreateBuffer(CreateInfo, Name);
	}

	inline uint32 RHIComputeMemorySize(RHITexture* TextureRHI)
	{
		return FDynamicRHI::Get()->RHIComputeMemorySize(TextureRHI);
	}

	inline RHISemaphoreRef RHICreateSemaphore()
	{
		return FDynamicRHI::Get()->RHICreateSemaphore();
	}

	inline RHICommandList* RHICreateGfxCommandList()
	{
		return FDynamicRHI::Get()->RHICreateGfxCommandList();
	}

	inline RHICommandList* RHICreateComputeCommandList()
	{
		return FDynamicRHI::Get()->RHICreateComputeCommandList();
	}

	inline RHICommandList* RHICreateTransferCommandList()
	{
		return FDynamicRHI::Get()->RHICreateTransferCommandList();
	}

	inline void RHISubmitCommandList(RHICommandList* RHICmdList, const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal)
	{
		FDynamicRHI::Get()->RHISubmitCommandList(RHICmdList, SemaphoresToWait, SemaphoresToSignal);
	}
}