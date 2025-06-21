#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include "VulkanDynamicRHI.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "Templates/AlignmentTemplates.h"
#include "VulkanCommandBuffer.h"
#include "VulkanBarriers.h"
#include "Common/Crc.h"

namespace nilou {

static VkFilter TranslateFilterModeToVkFilter(ETextureFilters Filter)
{
    switch (Filter) 
    {
    case TF_Linear:
    case TF_Linear_Mipmap_Linear:
    case TF_Linear_Mipmap_Nearest: 
        return VK_FILTER_LINEAR;
    case TF_Nearest:
    case TF_Nearest_Mipmap_Linear:
    case TF_Nearest_Mipmap_Nearest: 
        return VK_FILTER_NEAREST;
    }
    return VK_FILTER_MAX_ENUM;
}

static VkSamplerMipmapMode TranslateFilterModeToVkMipmapMode(ETextureFilters Filter)
{
    switch (Filter) 
    {
    case TF_Linear:
    case TF_Linear_Mipmap_Linear:
    case TF_Nearest_Mipmap_Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    case TF_Nearest:
    case TF_Linear_Mipmap_Nearest: 
    case TF_Nearest_Mipmap_Nearest: 
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
    return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
}

static VkSamplerAddressMode TranslateWrapMode(ETextureWrapModes WrapMode)
{
    switch (WrapMode) 
    {
    case TW_Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TW_Clamp:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TW_Mirrored_Repeat:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
    return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
}

static VkImageUsageFlags TranslateTextureUsageToVkUsageFlags(ETextureUsageFlags Usage)
{
    return (VkImageUsageFlags)Usage;
}

VulkanTexture::VulkanTexture(
        VkImage InImage,
        VkDeviceMemory InMemory,
        VkImageAspectFlags InAspectFlags,
        uint32 InSizeX,
        uint32 InSizeY,
        uint32 InSizeZ,
        uint32 InNumMips,
        uint32 InBaseMipLevel,
        uint32 InBaseArrayLayer,
        EPixelFormat InFormat,
        const std::string &InTextureName,
        ETextureDimension InTextureType)
    : RHITexture(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName, InTextureType)
    , Handle(InImage)
    , Memory(InMemory)
    , FullAspectFlags(InAspectFlags)
    , BaseArrayLayer(InBaseArrayLayer)
    , BaseMipLevel(InBaseMipLevel)
{ 

}

VulkanTexture::~VulkanTexture()
{
    if (Handle)
    {
        vkDestroyImage(Device->Handle, Handle, nullptr);
        Handle = VK_NULL_HANDLE;
    }
    if (Memory)
    {
        assert(Device->MemoryManager != nullptr);
        Device->MemoryManager->FreeMemory(Memory);
        Memory = VK_NULL_HANDLE;
    }
}

RHITextureRef FVulkanDynamicRHI::RHICreateTexture(const FRHITextureCreateInfo& CreateInfo, const std::string& Name)
{
    VkImageViewCreateInfo viewInfo{};
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.extent.width = CreateInfo.SizeX;
    imageInfo.extent.height = CreateInfo.SizeY;
    imageInfo.mipLevels = CreateInfo.NumMips;
    if (CreateInfo.TextureType == ETextureDimension::Texture2D)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.depth = 1;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
    else if (CreateInfo.TextureType == ETextureDimension::Texture2DArray)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = CreateInfo.SizeZ;
        imageInfo.extent.depth = 1;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    else if (CreateInfo.TextureType == ETextureDimension::Texture3D)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.depth = CreateInfo.SizeZ;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    }
    else if (CreateInfo.TextureType == ETextureDimension::TextureCube)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 6;
        imageInfo.extent.depth = 1;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    imageInfo.format = TranslatePixelFormatToVKFormat(CreateInfo.Format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
	imageInfo.usage = TranslateTextureUsageToVkUsageFlags(CreateInfo.Usage);

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkImage Image{};
    VkDeviceMemory Memory{};

    VK_CHECK_RESULT(vkCreateImage(Device->Handle, &imageInfo, nullptr, &Image));

    Device->MemoryManager->AllocateImageMemory(&Memory, Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkBindImageMemory(Device->Handle, Image, Memory, 0);

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Image;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = CreateInfo.NumMips;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;

    viewInfo.subresourceRange.aspectMask = GetAspectMaskFromPixelFormat(CreateInfo.Format, false, true);

    if (CreateInfo.TextureType == ETextureDimension::Texture2D)
    {
        return new VulkanTexture(
            Image, 
            Memory, 
            GetFullAspectMask(CreateInfo.Format), 
            CreateInfo.SizeX, CreateInfo.SizeY, 1, 
            CreateInfo.NumMips, 0, 0, 
            CreateInfo.Format, Name, ETextureDimension::Texture2D);
    }
    else if (CreateInfo.TextureType == ETextureDimension::Texture2DArray)
    {
        return new VulkanTexture(
            Image, 
            Memory, 
            GetFullAspectMask(CreateInfo.Format), 
            CreateInfo.SizeX, CreateInfo.SizeY, CreateInfo.SizeZ, 
            CreateInfo.NumMips, 0, 0, 
            CreateInfo.Format, Name, ETextureDimension::Texture2DArray);
    }
    else if (CreateInfo.TextureType == ETextureDimension::Texture3D)
    {
        return new VulkanTexture(
            Image, 
            Memory, 
            GetFullAspectMask(CreateInfo.Format), 
            CreateInfo.SizeX, CreateInfo.SizeY, CreateInfo.SizeZ, 
            CreateInfo.NumMips, 0, 0, 
            CreateInfo.Format, Name, ETextureDimension::Texture3D);
    }
    else if (CreateInfo.TextureType == ETextureDimension::TextureCube)
    {
        return new VulkanTexture(
            Image, 
            Memory, 
            GetFullAspectMask(CreateInfo.Format), 
            CreateInfo.SizeX, CreateInfo.SizeY, 6, 
            CreateInfo.NumMips, 0, 0, 
            CreateInfo.Format, Name, ETextureDimension::TextureCube);
    }
    return nullptr;
}

RHITextureViewRef FVulkanDynamicRHI::RHICreateTextureView(RHITexture* InTexture, const FRHITextureViewCreateInfo& CreateInfo, const std::string& Name)
{
    VulkanTexture* Texture = ResourceCast(InTexture);
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    switch (CreateInfo.ViewType) 
    {
    case ETextureDimension::Texture2D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case ETextureDimension::Texture2DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        break;
    case ETextureDimension::Texture3D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case ETextureDimension::TextureCube:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        break;
    default:
        Ncheckf(false, "RHICreateTextureView not implemented view type");
    }
    viewInfo.subresourceRange.aspectMask = GetAspectMaskFromPixelFormat(CreateInfo.Format, false, true);
    viewInfo.image = Texture->Handle;
    viewInfo.format = TranslatePixelFormatToVKFormat(CreateInfo.Format);
    viewInfo.subresourceRange.baseMipLevel = CreateInfo.BaseMipLevel;
    viewInfo.subresourceRange.levelCount = CreateInfo.LevelCount;
    viewInfo.subresourceRange.baseArrayLayer = CreateInfo.BaseArrayLayer;
    viewInfo.subresourceRange.layerCount = CreateInfo.LayerCount;
    
    TRefCountPtr<VulkanTextureView> TextureView = new VulkanTextureView(CreateInfo, InTexture);
    VK_CHECK_RESULT(vkCreateImageView(Device->Handle, &viewInfo, nullptr, &TextureView->Handle));

    return TextureView;

}

}