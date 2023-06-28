#pragma once
#include <array>
#include <tuple>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>

#include "DynamicRHI.h"
#include "VulkanResources.h"

namespace nilou {

class FVulkanCommandBufferManager;
class FVulkanMemoryManager;
class FVulkanStagingManager;

}

namespace nilou {
    
class FVulkanDynamicRHI : public FDynamicRHI
{
public:
    FVulkanDynamicRHI(const GfxConfiguration& Config);
    virtual int Initialize() override;
    virtual void Finalize() override;
    virtual void GetError(const char *file, int line) override;
    virtual EGraphicsAPI GetCurrentGraphicsAPI() { return EGraphicsAPI::Vulkan; }

    virtual void RHIBeginFrame() override;
    virtual void RHIEndFrame() override;

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
    // virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, int32 Value) override { return true; }
    // virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, int32 Value) override { return true; }
    // virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, float Value) override { return true; }
    // virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, float Value) override { return true; }
    // virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, uint32 Value) override { return true; }
    // virtual bool RHISetShaderUniformValue(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, uint32 Value) override { return true; }
    
    /**
    * Binding buffers
    */
    virtual void RHIBindComputeBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIBuffer* buffer) override { }
    virtual void RHIBindComputeBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIBuffer* buffer) override { }
    virtual void RHIBindFramebuffer(RHIFramebuffer *framebuffer) override { }
    virtual void RHIBindBufferData(RHIBuffer* buffer, unsigned int size, void *data) override;

    /**
    * Create/Update data
    */
    virtual FRHIGraphicsPipelineState *RHIGetOrCreatePipelineStateObject(const FGraphicsPipelineStateInitializer &Initializer) override;
    virtual RHIDepthStencilStateRef RHICreateDepthStencilState(const FDepthStencilStateInitializer &Initializer) override;
    virtual RHIRasterizerStateRef RHICreateRasterizerState(const FRasterizerStateInitializer &Initializer) override;
    virtual RHIBlendStateRef RHICreateBlendState(const FBlendStateInitializer &Initializer) override;
    virtual RHIVertexShaderRef RHICreateVertexShader(const std::string& code) override;
    virtual RHIPixelShaderRef RHICreatePixelShader(const std::string& code) override;
    virtual RHIComputeShaderRef RHICreateComputeShader(const std::string& code) override;
    virtual void RHIDestroyShader(RHIShader* Shader) override;
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
        int32 NumMips, uint32 InSizeX, uint32 InSizeY) override { return nullptr; }

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
    virtual void RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo) override;
    virtual void RHIDrawArrays(uint32 First, uint32 Count, int32 InstanceCount = 1) override { }
    virtual void RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount = 1) override;
    virtual void RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset = 0) override;
    virtual void RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override;
    virtual void RHIDispatchIndirect(RHIBuffer *indirectArgs, uint32 IndirectOffset = 1) override;
    virtual void RHIEndRenderPass() override;

    /**
    * Utils
    */
    virtual void RHIGenerateMipmap(RHITextureRef texture) override;
    virtual void *RHILockBuffer(RHIBuffer* buffer, uint32 Offset, uint32 Size, EResourceLockMode LockMode) override;
    virtual void RHIUnlockBuffer(RHIBuffer* buffer) override;
    virtual unsigned char *RHIReadImagePixel(RHITexture2DRef texture) override { return nullptr; }
    virtual void RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size) override;
    virtual void RHIImageMemoryBarrier() override { }
    virtual void RHIStorageMemoryBarrier() override { }
    virtual void RHIClearBuffer(uint32 flagbits) override { }
    virtual void RHISparseTextureUnloadTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel) override { }
    virtual void RHISparseTextureUpdateTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel, void* Data) override { }

    FVulkanCommandBufferManager* GetCommandBufferManager() const { return CommandBufferManager; }
    VkDevice device{};
    VkPhysicalDeviceProperties GpuProps;
    FVulkanCommandBufferManager* CommandBufferManager;
    FVulkanMemoryManager* MemoryManager;
    FVulkanStagingManager* StagingManager;
    FVulkanLayoutManager* LayoutManager;

    static VkFormat TranslatePixelFormatToVKFormat(EPixelFormat Format);
    
private:

    std::pair<VkShaderModule, shaderc_compilation_result_t> 
    RHICompileShaderInternal(const std::string& code, shaderc_shader_kind shader_kind);
    RHITextureRef RHICreateTextureInternal(
        const std::string &name, EPixelFormat Format, 
        int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureType TextureType);
    FRHIGraphicsPipelineStateRef RHICreateGraphicsPSO(const FGraphicsPipelineStateInitializer &Initializer);
    FRHIGraphicsPipelineStateRef RHICreateComputePSO(const FGraphicsPipelineStateInitializer &Initializer);
    std::shared_ptr<class VulkanPipelineLayout> RHICreatePipelineLayout(const FGraphicsPipelineStateInitializer& Initializer);
    void RHICreateBufferInternal(VkDevice Device, VkBufferUsageFlags UsageFlags, uint32 Size, void *Data, VkBuffer* Buffer, VkDeviceMemory* Memory);
    void RHIUpdateTextureInternal(
        RHITexture* Texture, void* Data, int32 MipmapLevel, 
        int32 Xoffset, int32 Yoffset, int32 Zoffset, uint32 Width, uint32 Height, uint32 Depth,
        int32 BaseArrayLayer);

    VkInstance instance{};
    VkSurfaceKHR surface{};
    VkPhysicalDevice physicalDevice{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    VkSwapchainKHR swapChain{};
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat{};
    VkFormat depthImageFormat{};
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews;
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers;
    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets;
    uint32 MemoryTypeIndex;
    FVulkanRenderPass* CurrentRenderPass;
    class VulkanFramebuffer* CurrentFramebuffer;

    uint8 currentFrame = 0;
    class shaderc_compiler* shader_compiler = nullptr;

    VkFormatProperties FormatProperties[PF_PixelFormatNum];

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
    uint32 GetFramesInFlight() const { return swapChainImages.size(); }

};

}