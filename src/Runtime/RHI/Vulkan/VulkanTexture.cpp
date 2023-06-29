#include "VulkanTexture.h"
#include "VulkanDynamicRHI.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "Templates/AlignmentTemplates.h"
#include "VulkanCommandBuffer.h"
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
    case TF_Nearest:
    case TF_Linear_Mipmap_Linear:
    case TF_Nearest_Mipmap_Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
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

VulkanTextureBase::~VulkanTextureBase()
{
    FVulkanDynamicRHI* RHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI());
    if (Image)
        vkDestroyImage(RHI->device, Image, nullptr);
    if (Memory)
        RHI->MemoryManager->FreeMemory(Memory);
    if (ImageView)
        vkDestroyImageView(RHI->device, ImageView, nullptr);
}

RHITexture2DRef FVulkanDynamicRHI::RHICreateTexture2D(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY)
{
    VulkanTexture2DRef Texture = std::static_pointer_cast<VulkanTexture2D>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, 1, ETextureType::TT_Texture2D));
    return Texture;
}

RHITexture2DArrayRef FVulkanDynamicRHI::RHICreateTexture2DArray(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ)
{
    VulkanTexture2DArrayRef Texture = std::static_pointer_cast<VulkanTexture2DArray>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, 1, ETextureType::TT_Texture2DArray));
    return Texture;
}

RHITexture3DRef FVulkanDynamicRHI::RHICreateTexture3D(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ)
{
    VulkanTexture3DRef Texture = std::static_pointer_cast<VulkanTexture3D>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, InSizeY, ETextureType::TT_Texture3D));
    return Texture;
}

RHITextureCubeRef FVulkanDynamicRHI::RHICreateTextureCube(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY)
{
    VulkanTextureCubeRef Texture = std::static_pointer_cast<VulkanTextureCube>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, 6, ETextureType::TT_TextureCube));
    return Texture;
}

RHISamplerStateRef FVulkanDynamicRHI::RHICreateSamplerState(const RHITextureParams& Params)
{

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = TranslateFilterModeToVkFilter(Params.Mag_Filter);
    samplerInfo.minFilter = TranslateFilterModeToVkFilter(Params.Min_Filter);;
    samplerInfo.addressModeU = TranslateWrapMode(Params.Wrap_S);
    samplerInfo.addressModeV = TranslateWrapMode(Params.Wrap_T);
    samplerInfo.addressModeW = TranslateWrapMode(Params.Wrap_R);
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = GpuProps.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = TranslateFilterModeToVkMipmapMode(Params.Min_Filter);
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = FLT_MAX;
    samplerInfo.mipLodBias = 0.0f;

    uint32 CRC = FCrc::MemCrc32(&samplerInfo, sizeof(samplerInfo));
    auto Found = SamplerMap.find(CRC);
    if (Found != SamplerMap.end())
        return Found->second;

    VulkanSamplerStateRef RHI = std::make_shared<VulkanSamplerState>(device);
    vkCreateSampler(device, &samplerInfo, nullptr, &RHI->Handle);
    SamplerMap[CRC] = RHI;
    return RHI;
}

RHITextureRef FVulkanDynamicRHI::RHICreateTextureInternal(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureType TextureType)
{
    VkImageViewCreateInfo viewInfo{};
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.extent.width = InSizeX;
    imageInfo.extent.height = InSizeY;
    imageInfo.mipLevels = NumMips;
    if (TextureType == ETextureType::TT_Texture2D)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.depth = 1;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
    else if (TextureType == ETextureType::TT_Texture2DArray)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = InSizeZ;
        imageInfo.extent.depth = 1;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    else if (TextureType == ETextureType::TT_Texture3D)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.depth = InSizeZ;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    }
    else if (TextureType == ETextureType::TT_TextureCube)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 6;
        imageInfo.extent.depth = 1;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    imageInfo.format = TranslatePixelFormatToVKFormat(Format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage Image{};
    VkImageView ImageView{};
    VkDeviceMemory Memory{};

    if (vkCreateImage(device, &imageInfo, nullptr, &Image) != VK_SUCCESS) {
        NILOU_LOG(Error, "failed to create image!")
        return nullptr;
    }

    MemoryManager->AllocateImageMemory(&Memory, Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkBindImageMemory(device, Image, Memory, 0);

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Image;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = NumMips;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;

    switch (Format) 
    {
    case EPixelFormat::PF_D32F:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    case EPixelFormat::PF_D24S8:
    case EPixelFormat::PF_D32FS8:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    default:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);

    if (TextureType == ETextureType::TT_Texture2D)
    {
        return std::make_shared<VulkanTexture2D>(Image, ImageView, Memory, imageInfo.initialLayout, InSizeX, InSizeY, 1, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_Texture2DArray)
    {
        return std::make_shared<VulkanTexture2DArray>(Image, ImageView, Memory, imageInfo.initialLayout, InSizeX, InSizeY, InSizeZ, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_Texture3D)
    {
        return std::make_shared<VulkanTexture3D>(Image, ImageView, Memory, imageInfo.initialLayout, InSizeX, InSizeY, InSizeZ, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_TextureCube)
    {
        return std::make_shared<VulkanTextureCube>(Image, ImageView, Memory, imageInfo.initialLayout, InSizeX, InSizeY, 6, NumMips, Format, name);
    }
    return nullptr;

}

RHITexture2DRef FVulkanDynamicRHI::RHICreateTextureView2D(RHITexture* OriginTexture, EPixelFormat Format, uint32 MinLevel, uint32 NumLevels, uint32 LevelIndex)
{
    VulkanTexture* Texture = static_cast<VulkanTexture*>(OriginTexture);
    VkImageViewCreateInfo viewInfo{};
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    switch (Format) 
    {
    case EPixelFormat::PF_D32F:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    case EPixelFormat::PF_D24S8:
    case EPixelFormat::PF_D32FS8:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    default:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageView ImageView{};

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Texture->GetImage();
    viewInfo.format = TranslatePixelFormatToVKFormat(Format);
    viewInfo.subresourceRange.baseMipLevel = MinLevel;
    viewInfo.subresourceRange.levelCount = NumLevels;
    viewInfo.subresourceRange.baseArrayLayer = LevelIndex;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);

    return std::make_shared<VulkanTexture2D>(
        VK_NULL_HANDLE, ImageView, VK_NULL_HANDLE, 
        Texture->GetSizeXYZ().x, Texture->GetSizeXYZ().y, 1, 
        NumLevels, Format, OriginTexture->GetName()+"_View");

}

RHITextureCubeRef FVulkanDynamicRHI::RHICreateTextureViewCube(RHITexture* OriginTexture, EPixelFormat Format, uint32 MinLevel, uint32 NumLevels)
{
    VulkanTexture* Texture = static_cast<VulkanTexture*>(OriginTexture);
    VkImageViewCreateInfo viewInfo{};
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

    switch (Format) 
    {
    case EPixelFormat::PF_D32F:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    case EPixelFormat::PF_D24S8:
    case EPixelFormat::PF_D32FS8:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    default:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageView ImageView{};

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Texture->GetImage();
    viewInfo.format = TranslatePixelFormatToVKFormat(Format);
    viewInfo.subresourceRange.baseMipLevel = MinLevel;
    viewInfo.subresourceRange.levelCount = NumLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);

    return std::make_shared<VulkanTextureCube>(
        VK_NULL_HANDLE, ImageView, VK_NULL_HANDLE, 
        Texture->GetSizeXYZ().x, Texture->GetSizeXYZ().y, 6, 
        NumLevels, Format, OriginTexture->GetName()+"_View");

}

static VkPipelineStageFlags GetVkStageFlagsForLayout(VkImageLayout Layout)
{
	VkPipelineStageFlags Flags = 0;

	switch (Layout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			Flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			break;

		case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
			Flags = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
			break;

		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
			Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
			break;
			
		case VK_IMAGE_LAYOUT_GENERAL:
		case VK_IMAGE_LAYOUT_UNDEFINED:
			Flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;

		case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
			// todo-jn: sync2 currently only used by depth/stencil targets
			Flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
			// todo-jn: sync2 currently only used by depth/stencil targets
			Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		default:
			break;
	}

	return Flags;
}

static VkAccessFlags GetVkAccessMaskForLayout(const VkImageLayout Layout)
{
	VkAccessFlags Flags = 0;

	switch (Layout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			Flags = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			Flags = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			Flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			Flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			Flags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			Flags = VK_ACCESS_SHADER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			Flags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			Flags = 0;
			break;

		case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
			Flags = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
			break;

		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
			Flags = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
			break;

		case VK_IMAGE_LAYOUT_GENERAL:
			// todo-jn: could be used for R64 in read layout
		case VK_IMAGE_LAYOUT_UNDEFINED:
			Flags = 0;
			break;

		case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
			// todo-jn: sync2 currently only used by depth/stencil targets
			Flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
			// todo-jn: sync2 currently only used by depth/stencil targets
			Flags = VK_ACCESS_SHADER_READ_BIT;
			break;

		default:
			break;
	}

	return Flags;
}

static void TransitionImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageLayout SrcLayout, VkImageLayout DstLayout, const VkImageSubresourceRange& SubresourceRange)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = SrcLayout;
    barrier.newLayout = DstLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = Image;
    barrier.subresourceRange = SubresourceRange;
    barrier.srcAccessMask = GetVkAccessMaskForLayout(SrcLayout);
    barrier.dstAccessMask = GetVkAccessMaskForLayout(DstLayout);

    VkPipelineStageFlags sourceStage = GetVkStageFlagsForLayout(SrcLayout);
    VkPipelineStageFlags destinationStage = GetVkStageFlagsForLayout(DstLayout);

    vkCmdPipelineBarrier(
        CmdBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

VkImageSubresourceRange MakeSubresourceRange(
    VkImageAspectFlags aspectMask, 
    uint32 baseMipLevel, 
    uint32 levelCount, 
    uint32 baseArrayLayer, 
    uint32 layerCount)
{
    VkImageSubresourceRange Range;
    Range.aspectMask = aspectMask;
    Range.baseMipLevel = baseMipLevel;
    Range.levelCount = levelCount;
    Range.baseArrayLayer = baseArrayLayer;
    Range.layerCount = layerCount;
    return Range;
}

void FVulkanDynamicRHI::RHIUpdateTextureInternal(
    RHITexture* Texture, void* Data, int32 MipmapLevel, 
    int32 Xoffset, int32 Yoffset, int32 Zoffset, uint32 Width, uint32 Height, uint32 Depth,
    int32 BaseArrayLayer)
{
    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(Texture);
    uint32 Size = Width * Height * Depth * TranslatePixelFormatToBytePerPixel(vkTexture->GetFormat());
    const VkPhysicalDeviceLimits& Limits = GpuProps.limits;
    uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(vkTexture->GetFormat());

    uint32 DataSize = Align(Size, Limits.minMemoryMapAlignment);
    FStagingBuffer* StagingBuffer = StagingManager->AcquireBuffer(DataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    void* Memory = StagingBuffer->GetMappedPointer();
    std::memcpy(Memory, Data, Size);

    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetUploadCmdBuffer();
    
	VkBufferImageCopy Region{};
	Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	Region.imageSubresource.mipLevel = MipmapLevel;
	Region.imageSubresource.baseArrayLayer = BaseArrayLayer;
	Region.imageSubresource.layerCount = 1;
	Region.imageOffset.x = Xoffset;
	Region.imageOffset.y = Yoffset;
	Region.imageOffset.z = Zoffset;
	Region.imageExtent.width = Width;
	Region.imageExtent.height = Height;
	Region.imageExtent.depth = Depth;

    TransitionImageLayout(
        CmdBuffer->GetHandle(), vkTexture->GetImage(), 
        vkTexture->GetImageLayout(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        MakeSubresourceRange(
            Region.imageSubresource.aspectMask, 
            Region.imageSubresource.mipLevel, 1, 
            BaseArrayLayer, 1));

    vkCmdCopyBufferToImage(
        CmdBuffer->GetHandle(), StagingBuffer->Buffer, 
        vkTexture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

    TransitionImageLayout(
        CmdBuffer->GetHandle(), vkTexture->GetImage(), 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
        MakeSubresourceRange(
            Region.imageSubresource.aspectMask, 
            Region.imageSubresource.mipLevel, 1, 
            BaseArrayLayer, 1));
    vkTexture->SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    CommandBufferManager->SubmitUploadCmdBuffer();

    StagingManager->ReleaseBuffer(CmdBuffer, StagingBuffer);
}

void FVulkanDynamicRHI::RHIUpdateTexture2D(RHITexture2D* Texture, 
    int32 Xoffset, int32 Yoffset, 
    int32 Width, int32 Height, 
    int32 MipmapLevel, void* Data)
{
    RHIUpdateTextureInternal(
        Texture,
        Data, MipmapLevel, 
        Xoffset, Yoffset, 0, 
        Width, Height, 1, 0);
}

void FVulkanDynamicRHI::RHIUpdateTexture3D(RHITexture3D* Texture, 
    int32 Xoffset, int32 Yoffset, int32 Zoffset,
    int32 Width, int32 Height, int32 Depth, 
    int32 MipmapLevel, void* Data)
{
    RHIUpdateTextureInternal(
        Texture,
        Data, MipmapLevel, 
        Xoffset, Yoffset, Zoffset, 
        Width, Height, Depth, 0);
}

void FVulkanDynamicRHI::RHIUpdateTexture2DArray(RHITexture2DArray* Texture, 
    int32 Xoffset, int32 Yoffset, int32 LayerIndex,
    int32 Width, int32 Height,
    int32 MipmapLevel, void* Data)
{
    RHIUpdateTextureInternal(
        Texture,
        Data, MipmapLevel, 
        Xoffset, Yoffset, 0, 
        Width, Height, 1, LayerIndex);
}

void FVulkanDynamicRHI::RHIUpdateTextureCube(RHITextureCube* Texture, 
    int32 Xoffset, int32 Yoffset, int32 LayerIndex,
    int32 Width, int32 Height,
    int32 MipmapLevel, void* Data)
{
    RHIUpdateTextureInternal(
        Texture,
        Data, MipmapLevel, 
        Xoffset, Yoffset, 0, 
        Width, Height, 1, LayerIndex);
}

void FVulkanDynamicRHI::RHIGenerateMipmap(RHITextureRef Texture)
{
    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(Texture.get());

    int32 LayerCount = 1;
    if (Texture->GetTextureType() == ETextureType::TT_Texture2DArray)
        LayerCount = Texture->GetSizeXYZ().z;

    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetUploadCmdBuffer();

    VkFormatProperties formatProperties = FormatProperties[Texture->GetFormat()];

    int32 MipWidth = Texture->GetSizeXYZ().x;
    int32 MipHeight = Texture->GetSizeXYZ().y;
    int32 MipDepth = 1;
    if (Texture->GetTextureType() == ETextureType::TT_Texture3D)
        MipDepth = Texture->GetSizeXYZ().z;
        
    TransitionImageLayout(
        CmdBuffer->GetHandle(), vkTexture->GetImage(), 
        vkTexture->GetImageLayout(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, vkTexture->GetNumMips(), 0, LayerCount));

    for (int i = 0; i < Texture->GetNumMips()-1; i++)
    {
        VkImageSubresourceRange Range = MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, i, 1, 0, LayerCount);
        
        TransitionImageLayout(
            CmdBuffer->GetHandle(), vkTexture->GetImage(), 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
            Range);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {MipWidth, MipHeight, MipDepth};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = LayerCount;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { MipWidth > 1 ? MipWidth / 2 : 1, MipHeight > 1 ? MipHeight / 2 : 1, MipDepth > 1 ? MipDepth / 2 : 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i+1;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = LayerCount;

        vkCmdBlitImage(CmdBuffer->GetHandle(),
            vkTexture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            vkTexture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        TransitionImageLayout(
            CmdBuffer->GetHandle(), vkTexture->GetImage(), 
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
            Range);

        if (MipWidth > 1) MipWidth /= 2;
        if (MipHeight > 1) MipHeight /= 2;
        if (MipDepth > 1) MipDepth /= 2;
    }

    TransitionImageLayout(
        CmdBuffer->GetHandle(), vkTexture->GetImage(), 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
        MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, Texture->GetNumMips()-1, 1, 0, LayerCount));

    CommandBufferManager->SubmitUploadCmdBuffer();

    vkTexture->SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}

}