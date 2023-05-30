#pragma once
#include <array>
#include <tuple>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>

#include "DynamicRHI.h"
#include "RHIResources.h"

namespace nilou {

class FVulkanDynamicRHI : public FDynamicRHI
{
public:
    virtual int Initialize() override;
    virtual void Finalize() override;
    virtual void GetError(const char *file, int line) override;
    virtual EGraphicsAPI GetCurrentGraphicsAPI() { return EGraphicsAPI::Vulkan; }

    /**
    * Set state
    */
    virtual void RHISetViewport(int32 Width, int32 Height) override { }
    virtual FRHIGraphicsPipelineState *RHISetComputeShader(RHIComputeShader *ComputeShader) override { return nullptr; }
    virtual void RHISetGraphicsPipelineState(FRHIGraphicsPipelineState *NewState) override { }
    virtual bool RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, RHIUniformBuffer *) override { return true; }
    virtual bool RHISetShaderUniformBuffer(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, RHIUniformBuffer *) override { return true; }
    virtual bool RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, const std::string &ParameterName, const FRHISampler &SamplerRHI) override { return true; }
    virtual bool RHISetShaderSampler(FRHIGraphicsPipelineState *, EPipelineStage PipelineStage, int BaseIndex, const FRHISampler &SamplerRHI) override { return true; }
    virtual bool RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, const std::string &ParameterName, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) override { return true; }
    virtual bool RHISetShaderImage(FRHIGraphicsPipelineState *BoundPipelineState, EPipelineStage PipelineStage, int BaseIndex, RHITexture *, EDataAccessFlag AccessFlag = EDataAccessFlag::DA_ReadOnly) override { return true; }
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
    virtual void RHIBindBufferData(RHIBuffer* buffer, unsigned int size, void *data, EBufferUsageFlags usage) override { }

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
    virtual void RHIDestroyShader(RHIShader* Shader) override;
    virtual RHIBufferRef RHICreateBuffer(uint32 Stride, uint32 Size, EBufferUsageFlags InUsage, void *Data) override { return nullptr; }
    virtual RHIUniformBufferRef RHICreateUniformBuffer(uint32 Size, EUniformBufferUsage InUsage, void *Data) override { return nullptr; }
    virtual RHIBufferRef RHICreateShaderStorageBuffer(unsigned int DataByteLength, void *Data) override { return nullptr; }
    virtual RHIBufferRef RHICreateAtomicCounterBuffer(unsigned int Value) override { return nullptr; }
    virtual RHIBufferRef RHICreateDispatchIndirectBuffer(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override { return nullptr; }
    virtual RHIBufferRef RHICreateDrawElementsIndirectBuffer(
            int32 Count, uint32 instanceCount, uint32 firstIndex, uint32 baseVertex, uint32 baseInstance) override { return nullptr; }
    
    virtual RHITexture2DRef RHICreateTexture2D(
        const std::string &name, EPixelFormat Format, 
        int32 NumMips, uint32 InSizeX, uint32 InSizeY) override { return nullptr; }
    virtual RHITexture2DArrayRef RHICreateTexture2DArray(
        const std::string &name, EPixelFormat Format, 
        int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ) override { return nullptr; }
    virtual RHITexture3DRef RHICreateTexture3D(
        const std::string &name, EPixelFormat Format, 
        int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ) override { return nullptr; }
    virtual RHITextureCubeRef RHICreateTextureCube(
        const std::string &name, EPixelFormat Format, 
        int32 NumMips, uint32 InSizeX, uint32 InSizeY) override { return nullptr; }
    virtual RHITexture2DRef RHICreateSparseTexture2D(
        const std::string &name, EPixelFormat Format, 
        int32 NumMips, uint32 InSizeX, uint32 InSizeY) override { return nullptr; }

    virtual RHIFramebufferRef RHICreateFramebuffer() override { return nullptr; }
    virtual RHIFramebufferRef RHICreateFramebuffer(EFramebufferAttachment attachment, RHITexture2DRef texture) override { return nullptr; }
    virtual RHIFramebufferRef RHICreateFramebuffer(
        EFramebufferAttachment attachment, RHITexture2DArrayRef texture, unsigned int layer_index
    ) override { return nullptr; }
    virtual void RHIUpdateUniformBuffer(RHIUniformBufferRef, void *Data) override { }
    virtual void RHIUpdateBuffer(RHIBuffer* Buffer, uint32 Offset, uint32 Size, void *Data) override { }
    virtual RHITexture2DRef RHICreateTextureView2D(
        RHITexture* OriginTexture, EPixelFormat Format, uint32 MinLevel, uint32 NumLevels, uint32 LevelIndex
    ) override { return nullptr; }
    virtual RHITextureCubeRef RHICreateTextureViewCube(
        RHITexture* OriginTexture, EPixelFormat Format, uint32 MinMipLevel, uint32 NumMipLevels
    ) override { return nullptr; }

    virtual void RHIUpdateTexture2D(RHITexture2D* Texture, 
        int32 Xoffset, int32 Yoffset, 
        int32 Width, int32 Height, 
        int32 MipmapLevel, void* Data) override { }
    virtual void RHIUpdateTexture3D(RHITexture3D* Texture, 
        int32 Xoffset, int32 Yoffset, int32 Zoffset,
        int32 Width, int32 Height, int32 Depth, 
        int32 MipmapLevel, void* Data) override { }
    virtual void RHIUpdateTexture2DArray(RHITexture2DArray* Texture, 
        int32 Xoffset, int32 Yoffset, int32 LayerIndex,
        int32 Width, int32 Height,
        int32 MipmapLevel, void* Data) override { }
    virtual void RHIUpdateTextureCube(RHITextureCube* Texture, 
        int32 Xoffset, int32 Yoffset, int32 LayerIndex,
        int32 Width, int32 Height,
        int32 MipmapLevel, void* Data) override { }

    /**
    * Render pass
    */
    virtual void RHIBeginRenderPass(const FRHIRenderPassInfo &InInfo) override { }
    virtual void RHIDrawArrays(uint32 First, uint32 Count, int32 InstanceCount = 1) override { }
    virtual void RHIDrawIndexed(RHIBuffer *IndexBuffer, int32 InstanceCount = 1) override { }
    virtual void RHIDrawIndexedIndirect(RHIBuffer *IndexBuffer, RHIBuffer *IndirectBuffer, uint32 IndirectOffset = 0) override { }
    virtual void RHIDispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) override { }
    virtual void RHIDispatchIndirect(RHIBuffer *indirectArgs) override { }
    virtual void RHIEndRenderPass() override { }

    /**
    * Utils
    */
    virtual FRHIRenderQueryRef RHICreateRenderQuery() override { return nullptr; }
    virtual void RHIBeginRenderQuery(FRHIRenderQuery* RenderQuery) override { }
    virtual void RHIEndRenderQuery(FRHIRenderQuery* RenderQuery) override { }
    virtual void RHIGetRenderQueryResult(FRHIRenderQuery* RenderQuery) override { }
    virtual void RHIGenerateMipmap(RHITextureRef texture) override { }
    virtual void *RHIMapComputeBuffer(RHIBufferRef buffer, EDataAccessFlag access) override { return nullptr; }
    virtual void RHIUnmapComputeBuffer(RHIBufferRef buffer) override { }
    virtual unsigned char *RHIReadImagePixel(RHITexture2DRef texture) override { return nullptr; }
    virtual void RHICopyBufferSubData(RHIBufferRef readBuffer, RHIBufferRef writeBuffer, int32 readOffset, int32 writeOffset, int32 size) override { }
    virtual void RHIImageMemoryBarrier() override { }
    virtual void RHIStorageMemoryBarrier() override { }
    virtual void RHIClearBuffer(uint32 flagbits) override { }
    virtual void RHISparseTextureUnloadTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel) override { }
    virtual void RHISparseTextureUpdateTile(RHITexture* Texture, uint32 TileX, uint32 TileY, uint32 MipmapLevel, void* Data) override { }

    
private:

    std::pair<VkShaderModule, shaderc_compilation_result_t> 
    RHICompileShaderInternal(const std::string& code, shaderc_shader_kind shader_kind);

    VkInstance instance{};
    VkSurfaceKHR surface{};
    VkPhysicalDevice physicalDevice{};
    VkDevice device{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    VkSwapchainKHR swapChain{};
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews;

    class shaderc_compiler* shader_compiler = nullptr;

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

};

}