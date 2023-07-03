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

VulkanTextureBase::VulkanTextureBase(
        VkImage InImage,
        VkImageView InImageView,
        VkDeviceMemory InMemory,
        const FVulkanImageLayout& InImageLayout)
    : Image(InImage)
    , ImageView(InImageView)
    , Memory(InMemory)
{ 
    GetLayoutManager().SetFullLayout(InImageView, InImageLayout);
}

VulkanTextureBase::~VulkanTextureBase()
{
    FVulkanDynamicRHI* RHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI());
    if (!IsImageView())
    {
        if (Image)
            vkDestroyImage(RHI->device, Image, nullptr);
        if (Memory)
            RHI->MemoryManager->FreeMemory(Memory);
    }
    if (ImageView)
        vkDestroyImageView(RHI->device, ImageView, nullptr);
}

const FVulkanImageLayout* VulkanTextureBase::GetImageLayout() const
{
    return GetLayoutManager().GetFullLayout(ImageView);
}

void VulkanTextureBase::SetImageLayout(VkImageLayout Layout, const VkImageSubresourceRange& Range)
{
    GetLayoutManager().UpdateLayout(ImageView, Range, Layout);
}

void VulkanTextureBase::SetFullImageLayout(VkImageLayout Layout)
{
    FVulkanImageLayout NewLayout = *GetImageLayout();
    NewLayout.MainLayout = Layout;
    NewLayout.SubresLayouts.clear();
    GetLayoutManager().SetFullLayout(ImageView, NewLayout);
}

RHITexture2DRef FVulkanDynamicRHI::RHICreateTexture2D(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags)
{
    VulkanTexture2DRef Texture = std::static_pointer_cast<VulkanTexture2D>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, 1, ETextureType::TT_Texture2D, InTexCreateFlags));
    return Texture;
}

RHITexture2DArrayRef FVulkanDynamicRHI::RHICreateTexture2DArray(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags)
{
    VulkanTexture2DArrayRef Texture = std::static_pointer_cast<VulkanTexture2DArray>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, InSizeZ, ETextureType::TT_Texture2DArray, InTexCreateFlags));
    return Texture;
}

RHITexture3DRef FVulkanDynamicRHI::RHICreateTexture3D(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureCreateFlags InTexCreateFlags)
{
    VulkanTexture3DRef Texture = std::static_pointer_cast<VulkanTexture3D>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, InSizeZ, ETextureType::TT_Texture3D, InTexCreateFlags));
    return Texture;
}

RHITextureCubeRef FVulkanDynamicRHI::RHICreateTextureCube(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, ETextureCreateFlags InTexCreateFlags)
{
    VulkanTextureCubeRef Texture = std::static_pointer_cast<VulkanTextureCube>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, 6, ETextureType::TT_TextureCube, InTexCreateFlags));
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
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureType TextureType, ETextureCreateFlags InTexCreateFlags)
{
    VkImageViewCreateInfo viewInfo{};
    VkImageCreateInfo imageInfo{};
    VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    imageInfo.format = TranslatePixelFormatToVKFormat(Format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = ImageLayout;
    
	imageInfo.usage = 0;
	imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	//@TODO: should everything be created with the source bit?
	imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if (EnumHasAnyFlags(InTexCreateFlags, TexCreate_Presentable))
	{
		imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;		
	}
	else if (EnumHasAnyFlags(InTexCreateFlags, TexCreate_RenderTargetable | TexCreate_DepthStencilTargetable))
	{
		if (EnumHasAllFlags(InTexCreateFlags, TexCreate_InputAttachmentRead))
		{
			imageInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		}
		imageInfo.usage |= (EnumHasAnyFlags(InTexCreateFlags, TexCreate_RenderTargetable) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		// if (EnumHasAllFlags(InTexCreateFlags, TexCreate_Memoryless) && InDevice.GetDeviceMemoryManager().SupportsMemoryless())
		// {
		// 	imageInfo.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		// 	// Remove the transfer and sampled bits, as they are incompatible with the transient bit.
		// 	imageInfo.usage &= ~(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		// }
	}
	else if (EnumHasAnyFlags(InTexCreateFlags, TexCreate_DepthStencilResolveTarget))
	{
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else if (EnumHasAnyFlags(InTexCreateFlags, TexCreate_ResolveTargetable))
	{
		imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	
	if (EnumHasAnyFlags(InTexCreateFlags, TexCreate_UAV))
	{
		//cannot have the storage bit on a memoryless texture
		assert(!EnumHasAnyFlags(InTexCreateFlags, TexCreate_Memoryless));
		imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

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

    viewInfo.subresourceRange.aspectMask = GetAspectMaskFromPixelFormat(Format, true, true);

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);
    
    // if (Format == PF_D32F || Format == PF_D24S8 || Format == PF_D32FS8)
    // {
    //     FVulkanCmdBuffer* UploadCmdBuffer = CommandBufferManager->GetUploadCmdBuffer();
    //     ImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //     VkImageAspectFlags Aspect = GetFullAspectMask();
    //     if (Format == PF_D32F)
    //     {
    //         ImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    //         Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    //     }
    //     FVulkanPipelineBarrier Barrier;
    //     Barrier.AddImageLayoutTransition(
    //         Image, VK_IMAGE_LAYOUT_UNDEFINED, ImageLayout, 
    //         FVulkanPipelineBarrier::MakeSubresourceRange(Aspect, 
    //         0, NumMips, 0, imageInfo.arrayLayers));
    //     Barrier.Execute(UploadCmdBuffer);
    //     // TransitionImageLayout(
    //     //     UploadCmdBuffer->GetHandle(), Image, 
    //     //     VK_IMAGE_LAYOUT_UNDEFINED, ImageLayout,
    //     //     MakeSubresourceRange(Aspect, 
    //     //     0, NumMips, 0, imageInfo.arrayLayers));
    //     CommandBufferManager->SubmitUploadCmdBuffer();
    // }

    if (TextureType == ETextureType::TT_Texture2D)
    {
        FVulkanImageLayout Layout{ImageLayout, (uint32)NumMips, 1, GetFullAspectMask(Format)};
        return std::make_shared<VulkanTexture2D>(Image, ImageView, Memory, Layout, InSizeX, InSizeY, 1, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_Texture2DArray)
    {
        FVulkanImageLayout Layout{ImageLayout, (uint32)NumMips, InSizeZ, GetFullAspectMask(Format)};
        return std::make_shared<VulkanTexture2DArray>(Image, ImageView, Memory, Layout, InSizeX, InSizeY, InSizeZ, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_Texture3D)
    {
        FVulkanImageLayout Layout{ImageLayout, (uint32)NumMips, 1, GetFullAspectMask(Format)};
        return std::make_shared<VulkanTexture3D>(Image, ImageView, Memory, Layout, InSizeX, InSizeY, InSizeZ, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_TextureCube)
    {
        FVulkanImageLayout Layout{ImageLayout, (uint32)NumMips, 6, GetFullAspectMask(Format)};
        return std::make_shared<VulkanTextureCube>(Image, ImageView, Memory, Layout, InSizeX, InSizeY, 6, NumMips, Format, name);
    }
    return nullptr;

}

RHITexture2DRef FVulkanDynamicRHI::RHICreateTextureView2D(RHITexture* OriginTexture, EPixelFormat Format, uint32 MinLevel, uint32 NumLevels, uint32 LayerIndex)
{
    VulkanTextureBase* Texture = ResourceCast(OriginTexture);
    VkImageViewCreateInfo viewInfo{};
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.subresourceRange.aspectMask = GetAspectMaskFromPixelFormat(Format, true, true);

    VkImageView ImageView{};

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Texture->Image;
    viewInfo.format = TranslatePixelFormatToVKFormat(Format);
    viewInfo.subresourceRange.baseMipLevel = MinLevel;
    viewInfo.subresourceRange.levelCount = NumLevels;
    viewInfo.subresourceRange.baseArrayLayer = LayerIndex;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);

    const FVulkanImageLayout* Layout = Texture->GetImageLayout();
    FVulkanImageLayout ViewLayout = Layout->GetSubresLayout(viewInfo.subresourceRange);

    return std::make_shared<VulkanTextureView2D>(
        Texture, Texture->Image, ImageView, Texture->Memory, ViewLayout,
        OriginTexture->GetSizeXYZ().x >> MinLevel, OriginTexture->GetSizeXYZ().y >> MinLevel, 1, 
        MinLevel, NumLevels, LayerIndex, 1, Format, OriginTexture->GetName()+"_View");

}

RHITextureCubeRef FVulkanDynamicRHI::RHICreateTextureViewCube(RHITexture* OriginTexture, EPixelFormat Format, uint32 MinLevel, uint32 NumLevels)
{
    if (OriginTexture->GetTextureType() != ETextureType::TT_TextureCube)
        return nullptr;
    VulkanTextureBase* Texture = ResourceCast(OriginTexture);
    VkImageViewCreateInfo viewInfo{};
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.subresourceRange.aspectMask = GetFullAspectMask(OriginTexture->GetFormat());

    VkImageView ImageView{};

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Texture->Image;
    viewInfo.format = TranslatePixelFormatToVKFormat(Format);
    viewInfo.subresourceRange.baseMipLevel = MinLevel;
    viewInfo.subresourceRange.levelCount = NumLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);

    const FVulkanImageLayout* Layout = Texture->GetImageLayout();
    FVulkanImageLayout ViewLayout = Layout->GetSubresLayout(viewInfo.subresourceRange);

    return std::make_shared<VulkanTextureViewCube>(
        Texture, Texture->Image, ImageView, Texture->Memory, ViewLayout,
        OriginTexture->GetSizeXYZ().x >> MinLevel, OriginTexture->GetSizeXYZ().y >> MinLevel, 6, 
        MinLevel, NumLevels, 0, 6, Format, OriginTexture->GetName()+"_View");

}

void FVulkanDynamicRHI::RHIUpdateTextureInternal(
    RHITexture* Texture, void* Data, int32 MipmapLevel, 
    int32 Xoffset, int32 Yoffset, int32 Zoffset, uint32 Width, uint32 Height, uint32 Depth,
    int32 BaseArrayLayer)
{

    VulkanTextureBase* vkTexture = ResourceCast(Texture);
    uint32 Size = Width * Height * Depth * TranslatePixelFormatToBytePerPixel(Texture->GetFormat());
    const VkPhysicalDeviceLimits& Limits = GpuProps.limits;
    uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(Texture->GetFormat());

    uint32 DataSize = Align(Size, Limits.minMemoryMapAlignment);
    FStagingBuffer* StagingBuffer = StagingManager->AcquireBuffer(DataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    
    if (Data)
    {
        void* Memory = StagingBuffer->GetMappedPointer();
        std::memcpy(Memory, Data, Size);
    }

    FVulkanCmdBuffer* CmdBuffer = CommandBufferManager->GetUploadCmdBuffer();
    
	VkBufferImageCopy Region{};
	Region.imageSubresource.aspectMask = GetFullAspectMask(Texture->GetFormat());
	Region.imageSubresource.mipLevel = MipmapLevel;
	Region.imageSubresource.baseArrayLayer = BaseArrayLayer;
	Region.imageSubresource.layerCount = 1;
	Region.imageOffset.x = Xoffset;
	Region.imageOffset.y = Yoffset;
	Region.imageOffset.z = Zoffset;
	Region.imageExtent.width = Width;
	Region.imageExtent.height = Height;
	Region.imageExtent.depth = Depth;

    VkImageSubresourceRange SubresRange = FVulkanPipelineBarrier::MakeSubresourceRange(
        Region.imageSubresource.aspectMask, 
        MipmapLevel, 1, 
        BaseArrayLayer, 1);

    const FVulkanImageLayout* Layout = vkTexture->GetImageLayout();
    VkImageLayout PartialLayout = Layout->GetSubresLayout(BaseArrayLayer, MipmapLevel, 0);

    {
        FVulkanPipelineBarrier Barrier;
        Barrier.AddImageLayoutTransition(vkTexture->Image, 
            PartialLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
            SubresRange);
        Barrier.Execute(CmdBuffer);
    }

    vkCmdCopyBufferToImage(
        CmdBuffer->GetHandle(), StagingBuffer->Buffer, 
        vkTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

    {
        FVulkanPipelineBarrier Barrier;
        Barrier.AddImageLayoutTransition(vkTexture->Image, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
            SubresRange);
        Barrier.Execute(CmdBuffer);
    }

    vkTexture->SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, SubresRange);

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
    VulkanTextureBase* vkTexture = ResourceCast(Texture.get());

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
    
    {
        const FVulkanImageLayout* Layout = vkTexture->GetImageLayout();
        FVulkanPipelineBarrier Barrier;
        Barrier.AddImageLayoutTransition(
            vkTexture->Image, GetAspectMaskFromPixelFormat(Texture->GetFormat(), true, true),
            *Layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        Barrier.Execute(CmdBuffer);
    }

    for (int i = 0; i < Texture->GetNumMips()-1; i++)
    {
        VkImageSubresourceRange Range = FVulkanPipelineBarrier::MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, i, 1, 0, LayerCount);
        
        {
            FVulkanPipelineBarrier Barrier;
            Barrier.AddImageLayoutTransition(
                vkTexture->Image, 
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
                Range);
            Barrier.Execute(CmdBuffer);
        }
        // TransitionImageLayout(
        //     CmdBuffer->GetHandle(), vkTexture->Image, 
        //     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
        //     Range);

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
            vkTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            vkTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        {
            FVulkanPipelineBarrier Barrier;
            Barrier.AddImageLayoutTransition(
                vkTexture->Image, 
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
                Range);
            Barrier.Execute(CmdBuffer);
            vkTexture->SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Range);
        }
        // TransitionImageLayout(
        //     CmdBuffer->GetHandle(), vkTexture->Image, 
        //     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
        //     Range);

        if (MipWidth > 1) MipWidth /= 2;
        if (MipHeight > 1) MipHeight /= 2;
        if (MipDepth > 1) MipDepth /= 2;
    }

    {
        VkImageSubresourceRange Range = FVulkanPipelineBarrier::MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, Texture->GetNumMips()-1, 1, 0, LayerCount);
        FVulkanPipelineBarrier Barrier;
        Barrier.AddImageLayoutTransition(
            vkTexture->Image, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
            Range);
        Barrier.Execute(CmdBuffer);
        vkTexture->SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Range);
    }
    // TransitionImageLayout(
    //     CmdBuffer->GetHandle(), vkTexture->Image, 
    //     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
    //     MakeSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, Texture->GetNumMips()-1, 1, 0, LayerCount));

    CommandBufferManager->SubmitUploadCmdBuffer();

}

}