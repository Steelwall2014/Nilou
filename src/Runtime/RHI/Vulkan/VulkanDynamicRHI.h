#pragma once
#include <array>
#include <tuple>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>

#include "DynamicRHI.h"
#include "VulkanResources.h"
#include "Common/Containers/Array.h"

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult _res = (f);																				\
	if (_res != VK_SUCCESS)																				\
	{																									\
        NILOU_LOG(Fatal, "VkResult is \"{}\" in {} at line {}", FVulkanDynamicRHI::ErrorString(_res), __FILE__, __LINE__); \
	}																									\
}

namespace nilou {

class FVulkanCommandBufferManager;
class FVulkanMemoryManager;
class FVulkanStagingManager;
class VulkanQueue;
class VulkanTexture;
class VulkanDevice;
class VulkanCommandBufferPool;

}

namespace nilou {
    
class FVulkanDynamicRHI : public FDynamicRHI
{
public:
    static FVulkanDynamicRHI *GetDynamicRHI() { return static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI()); }
    FVulkanDynamicRHI(const GfxConfiguration& Config);
    virtual int Initialize() override;
    virtual void Finalize() override;
    virtual void GetError(const char *file, int line) override;
    virtual EGraphicsAPI GetCurrentGraphicsAPI() override { return EGraphicsAPI::Vulkan; }

    /**
    * Create/Update data
    */
    virtual RHIGraphicsPipelineState *RHICreateGraphicsPipelineState(const FGraphicsPipelineStateInitializer &Initializer, const std::vector<RHIPushConstantRange>& PushConstantRanges={}) override;
    virtual RHIComputePipelineState *RHICreateComputePipelineState(RHIComputeShader* ComputeShader, const std::vector<RHIPushConstantRange>& PushConstantRanges={}) override;
    virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) override;
    virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) override;
    virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) override;
    virtual RHISamplerStateRef RHICreateSamplerState(const FSamplerStateInitializer& Initializer) override;
    virtual RHIVertexShaderRef RHICreateVertexShader(const std::string& code) override;
    virtual RHIPixelShaderRef RHICreatePixelShader(const std::string& code) override;
    virtual RHIComputeShaderRef RHICreateComputeShader(const std::string& code) override;
    virtual RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, const void *Data) override;
    virtual RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data) override;
    virtual RHIBufferRef RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override;
    virtual RHIBufferRef RHICreateDrawElementsIndirectBuffer(
            int32 Count, uint32 instanceCount, uint32 firstIndex, uint32 baseVertex, uint32 baseInstance) override;
    
    // virtual RHITexture2DRef RHICreateTexture2D(
    //     const std::string &name, EPixelFormat Format, 
    //     int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) override;
    // virtual RHITexture2DArrayRef RHICreateTexture2DArray(
    //     const std::string &name, EPixelFormat Format, 
    //     int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags) override;
    // virtual RHITexture3DRef RHICreateTexture3D(
    //     const std::string &name, EPixelFormat Format, 
    //     int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags) override;
    // virtual RHITextureCubeRef RHICreateTextureCube(
    //     const std::string &name, EPixelFormat Format, 
    //     int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) override;
    // virtual RHITexture2DRef RHICreateSparseTexture2D(
    //     const std::string &name, EPixelFormat Format, 
    //     int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags) override { return nullptr; }
    virtual RHITextureRef RHICreateTexture(const FRHITextureCreateInfo& CreateInfo, const std::string& Name) override;

    // virtual RHIFramebufferRef RHICreateFramebuffer(const std::array<RHITextureView*, MaxSimultaneousRenderTargets>& InAttachments, RHITextureView* InDepthStencilAttachment) override;
    // virtual void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data) override;
    // virtual void RHIUpdateBuffer(RHIBuffer* Buffer, uint32 Offset, uint32 Size, void *Data) override;
    virtual RHITextureViewRef RHICreateTextureView(RHITexture* Texture, const FRHITextureViewCreateInfo& CreateInfo, const std::string& Name) override;
    virtual FRHIVertexDeclaration* RHICreateVertexDeclaration(const FVertexDeclarationElementList& Elements) override;

    virtual void* RHIMapMemory(RHIBuffer* buffer, uint32 Offset, uint32 Size) override;
    virtual void RHIUnmapMemory(RHIBuffer* buffer) override;
    virtual uint32 RHIComputeMemorySize(RHITexture* TextureRHI) override;

	virtual RHIDescriptorSetLayoutRef RHICreateDescriptorSetLayout(const std::vector<RHIDescriptorSetLayoutBinding>& Bindings) override;
    virtual RHIDescriptorPoolRef RHICreateDescriptorPool(RHIDescriptorSetLayout* Layout, uint32 PoolSize) override;
    virtual RHISemaphoreRef RHICreateSemaphore() override;
    virtual RHICommandList* RHICreateGfxCommandList() override;
    virtual RHICommandList* RHICreateComputeCommandList() override;
    virtual RHICommandList* RHICreateTransferCommandList() override;
    virtual void RHISubmitCommandList(RHICommandList* RHICmdList, const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal) override;

    VulkanDevice* Device = nullptr;
    VkPhysicalDeviceProperties GpuProps;
    std::unique_ptr<FVulkanRenderPassManager> RenderPassManager;

    static std::string ErrorString(VkResult Result);
    
private:

    std::pair<VkShaderModule, shaderc_compilation_result_t> 
    RHICompileShaderInternal(const std::string& code, shaderc_shader_kind shader_kind);
    RHITextureRef RHICreateTextureInternal(
        const std::string &name, EPixelFormat Format, 
        int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureDimension TextureType, ETextureCreateFlags InTexCreateFlags);
    RHIGraphicsPipelineStateRef RHICreateGraphicsPSO(const FGraphicsPipelineStateInitializer &Initializer, const std::vector<RHIPushConstantRange>& PushConstantRanges);
    RHIComputePipelineStateRef RHICreateComputePSO(RHIComputeShader* ComputeShader, const std::vector<RHIPushConstantRange>& PushConstantRanges);
    RHIPipelineLayoutRef RHICreatePipelineLayout(const std::vector<RHIShader*>& shaders, const std::vector<RHIPushConstantRange>& PushConstantRanges);
    void RHICreateBufferInternal(VkDevice Device, VkBufferUsageFlags UsageFlags, uint32 Size, void *Data, VkBuffer* Buffer, VkDeviceMemory* Memory);
    void RHIUpdateTextureInternal(
        RHITexture* Texture, void* Data, int32 MipmapLevel, 
        int32 Xoffset, int32 Yoffset, int32 Zoffset, uint32 Width, uint32 Height, uint32 Depth,
        int32 BaseArrayLayer);
    void TransitionImageLayout(RHITexture* Texture, VkImageLayout DstLayout);
    void TransitionImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageLayout SrcLayout, VkImageLayout DstLayout, const VkImageSubresourceRange& SubresourceRange);
    RHICommandList* RHICreateCommandList(VulkanCommandBufferPool* Pool);

    std::map<uint32, RHISamplerStateRef> SamplerMap;

    VkInstance instance{};
    VkSurfaceKHR surface{};
    VkPhysicalDevice physicalDevice{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    std::unique_ptr<class FVulkanSwapChain> SwapChain;
    std::vector<RHITextureRef> swapChainImages;
    std::vector<RHITextureViewRef> swapChainImageViews;
    EPixelFormat swapChainImageFormat;
    EPixelFormat depthImageFormat;
    VkExtent2D swapChainExtent{};
    RHITextureRef DepthImage;
    RHITextureViewRef DepthImageView;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkQueueFamilyProperties> queueFamilies;
    uint64 CurrentStreamSourceOffsets[MAX_VERTEX_ELEMENTS] = { 0 };
    VkBuffer CurrentStreamSourceBuffers[MAX_VERTEX_ELEMENTS] = { nullptr };
    std::vector<VkFence> FrameFences;
    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
    uint32 CurrentSwapChainImageIndex = 0;
    std::shared_ptr<FVulkanSemaphore> CurrentImageAcquiredSemaphore;

    FVulkanRenderPass* RenderToScreenPass{};
    

    class shaderc_compiler* shader_compiler = nullptr;

    VkFormatProperties FormatProperties[PF_MAX];

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        std::optional<uint32> graphicsFamily;
        std::optional<uint32> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);

    void PrepareForDispatch();
    void PrepareForDraw();

};

}