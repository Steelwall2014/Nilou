#include "Common/Crc.h"
#include "VulkanDynamicRHI.h"
#include "VulkanTexture.h"
#include "Common/Containers/Array.h"

namespace nilou {

static inline VkCompareOp CompareOpToVulkan(ECompareFunction InOp)
{
	switch (InOp)
	{
		case CF_Less:			return VK_COMPARE_OP_LESS;
		case CF_LessEqual:		return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CF_Greater:		return VK_COMPARE_OP_GREATER;
		case CF_GreaterEqual:	return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CF_Equal:			return VK_COMPARE_OP_EQUAL;
		case CF_NotEqual:		return VK_COMPARE_OP_NOT_EQUAL;
		case CF_Never:			return VK_COMPARE_OP_NEVER;
		case CF_Always:			return VK_COMPARE_OP_ALWAYS;
		default:
			break;
	}

	NILOU_LOG(Error, "Unknown ECompareFunction {}", magic_enum::enum_name(InOp));
	return VK_COMPARE_OP_MAX_ENUM;
}

static inline VkStencilOp StencilOpToVulkan(EStencilOp InOp)
{
	VkStencilOp OutOp = VK_STENCIL_OP_MAX_ENUM;

	switch (InOp)
	{
		case SO_Keep:					return VK_STENCIL_OP_KEEP;
		case SO_Zero:					return VK_STENCIL_OP_ZERO;
		case SO_Replace:				return VK_STENCIL_OP_REPLACE;
		case SO_SaturatedIncrement:		return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case SO_SaturatedDecrement:		return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case SO_Invert:					return VK_STENCIL_OP_INVERT;
		case SO_Increment:				return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case SO_Decrement:				return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		default:
			break;
	}

	NILOU_LOG(Error, "Unknown EStencilOp {}", magic_enum::enum_name(InOp));
	return VK_STENCIL_OP_MAX_ENUM;
}

static inline VkPolygonMode RasterizerFillModeToVulkan(ERasterizerFillMode InFillMode)
{
	switch (InFillMode)
	{
		case FM_Point:			return VK_POLYGON_MODE_POINT;
		case FM_Wireframe:		return VK_POLYGON_MODE_LINE;
		case FM_Solid:			return VK_POLYGON_MODE_FILL;
		default:
			break;
	}

	NILOU_LOG(Error, "Unknown ERasterizerFillMode {}", magic_enum::enum_name(InFillMode));
	return VK_POLYGON_MODE_MAX_ENUM;
}

static inline VkCullModeFlags RasterizerCullModeToVulkan(ERasterizerCullMode InCullMode)
{
	switch (InCullMode)
	{
		case CM_None:	return VK_CULL_MODE_NONE;
		case CM_CW:		return VK_CULL_MODE_FRONT_BIT;
		case CM_CCW:	return VK_CULL_MODE_BACK_BIT;
		default:		break;
	}

	NILOU_LOG(Error, "Unknown ERasterizerCullMode {}", magic_enum::enum_name(InCullMode));
	return VK_CULL_MODE_NONE;
}

static inline VkBlendOp BlendOpToVulkan(EBlendOperation InOp)
{
	switch (InOp)
	{
		case BO_Add:				return VK_BLEND_OP_ADD;
		case BO_Subtract:			return VK_BLEND_OP_SUBTRACT;
		case BO_Min:				return VK_BLEND_OP_MIN;
		case BO_Max:				return VK_BLEND_OP_MAX;
		case BO_ReverseSubtract:	return VK_BLEND_OP_REVERSE_SUBTRACT;
		default:
			break;
	}

	NILOU_LOG(Error, "Unknown EBlendOperation {}", magic_enum::enum_name(InOp));
	return VK_BLEND_OP_MAX_ENUM;
}

static inline VkBlendFactor BlendFactorToVulkan(EBlendFactor InFactor)
{
	switch (InFactor)
	{
		case BF_Zero:						return VK_BLEND_FACTOR_ZERO;
		case BF_One:						return VK_BLEND_FACTOR_ONE;
		case BF_SourceColor:				return VK_BLEND_FACTOR_SRC_COLOR;
		case BF_InverseSourceColor:			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BF_SourceAlpha:				return VK_BLEND_FACTOR_SRC_ALPHA;
		case BF_InverseSourceAlpha:			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BF_DestAlpha:					return VK_BLEND_FACTOR_DST_ALPHA;
		case BF_InverseDestAlpha:			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BF_DestColor:					return VK_BLEND_FACTOR_DST_COLOR;
		case BF_InverseDestColor:			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BF_ConstantBlendFactor:		return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case BF_InverseConstantBlendFactor:	return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case BF_Source1Color:				return VK_BLEND_FACTOR_SRC1_COLOR;
		case BF_InverseSource1Color:		return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case BF_Source1Alpha:				return VK_BLEND_FACTOR_SRC1_ALPHA;
		case BF_InverseSource1Alpha:		return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		default:
			break;
	}

	NILOU_LOG(Error, "Unknown EBlendFactor {}", magic_enum::enum_name(InFactor));
	return VK_BLEND_FACTOR_MAX_ENUM;
}

VulkanGraphicsPipelineState::~VulkanGraphicsPipelineState()
{
	vkDestroyPipeline(Device, Handle, nullptr);
}

VulkanDepthStencilState::VulkanDepthStencilState(const FDepthStencilStateInitializer& Initializer)
{
    DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencilState.depthTestEnable = (Initializer.DepthTest != CF_Always || Initializer.bEnableDepthWrite) ? VK_TRUE : VK_FALSE;
    DepthStencilState.depthCompareOp = CompareOpToVulkan(Initializer.DepthTest);
    DepthStencilState.depthWriteEnable = Initializer.bEnableDepthWrite ? VK_TRUE : VK_FALSE;

    DepthStencilState.depthBoundsTestEnable = VK_FALSE;
    DepthStencilState.minDepthBounds = 0.0f;
    DepthStencilState.maxDepthBounds = 1.0f;

    DepthStencilState.stencilTestEnable = (Initializer.bEnableFrontFaceStencil || Initializer.bEnableBackFaceStencil) ? VK_TRUE : VK_FALSE;

    // Front
    DepthStencilState.back.failOp = StencilOpToVulkan(Initializer.FrontFaceStencilFailStencilOp);
    DepthStencilState.back.passOp = StencilOpToVulkan(Initializer.FrontFacePassStencilOp);
    DepthStencilState.back.depthFailOp = StencilOpToVulkan(Initializer.FrontFaceDepthFailStencilOp);
    DepthStencilState.back.compareOp = CompareOpToVulkan(Initializer.FrontFaceStencilTest);
    DepthStencilState.back.compareMask = Initializer.StencilReadMask;
    DepthStencilState.back.writeMask = Initializer.StencilWriteMask;
    DepthStencilState.back.reference = 0;

    if (Initializer.bEnableBackFaceStencil)
    {
        // Back
        DepthStencilState.front.failOp = StencilOpToVulkan(Initializer.BackFaceStencilFailStencilOp);
        DepthStencilState.front.passOp = StencilOpToVulkan(Initializer.BackFacePassStencilOp);
        DepthStencilState.front.depthFailOp = StencilOpToVulkan(Initializer.BackFaceDepthFailStencilOp);
        DepthStencilState.front.compareOp = CompareOpToVulkan(Initializer.BackFaceStencilTest);
        DepthStencilState.front.compareMask = Initializer.StencilReadMask;
        DepthStencilState.front.writeMask = Initializer.StencilWriteMask;
        DepthStencilState.front.reference = 0;
    }
    else
    {
        DepthStencilState.front = DepthStencilState.back;
    }
}

VulkanRasterizerState::VulkanRasterizerState(const FRasterizerStateInitializer& Initializer)
{
    RasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizerState.polygonMode = RasterizerFillModeToVulkan(Initializer.FillMode);
	RasterizerState.cullMode = RasterizerCullModeToVulkan(Initializer.CullMode);
    RasterizerState.frontFace = VK_FRONT_FACE_CLOCKWISE;    // Set the front face to clockwise because we will use flipped viewport in vulkan

	//RasterizerState.depthClampEnable = VK_FALSE;
	// RasterizerState.depthBiasEnable = Initializer.DepthBias != 0.0f ? VK_TRUE : VK_FALSE;
	//RasterizerState.rasterizerDiscardEnable = VK_FALSE;

	// RasterizerState.depthBiasSlopeFactor = Initializer.SlopeScaleDepthBias;
	// RasterizerState.depthBiasConstantFactor = Initializer.DepthBias;
}

VulkanBlendState::VulkanBlendState(const FBlendStateInitializer& Initializer)
{
    for (int i = 0; i < MaxSimultaneousRenderTargets; i++)
    {
		auto& ColorTarget = Initializer.RenderTargets[i];
        auto& BlendState = BlendStates[i];
        BlendState = VkPipelineColorBlendAttachmentState{};
        BlendState.colorBlendOp = BlendOpToVulkan(ColorTarget.ColorBlendOp);
		BlendState.alphaBlendOp = BlendOpToVulkan(ColorTarget.AlphaBlendOp);

		BlendState.dstColorBlendFactor = BlendFactorToVulkan(ColorTarget.ColorDestBlend);
		BlendState.dstAlphaBlendFactor = BlendFactorToVulkan(ColorTarget.AlphaDestBlend);

		BlendState.srcColorBlendFactor = BlendFactorToVulkan(ColorTarget.ColorSrcBlend);
		BlendState.srcAlphaBlendFactor = BlendFactorToVulkan(ColorTarget.AlphaSrcBlend);

		BlendState.blendEnable =
			(ColorTarget.ColorBlendOp != BO_Add || ColorTarget.ColorDestBlend != BF_Zero || ColorTarget.ColorSrcBlend != BF_One ||
			ColorTarget.AlphaBlendOp != BO_Add || ColorTarget.AlphaDestBlend != BF_Zero || ColorTarget.AlphaSrcBlend != BF_One) ? VK_TRUE : VK_FALSE;

		BlendState.colorWriteMask = (ColorTarget.ColorWriteMask & CW_RED) ? VK_COLOR_COMPONENT_R_BIT : 0;
		BlendState.colorWriteMask |= (ColorTarget.ColorWriteMask & CW_GREEN) ? VK_COLOR_COMPONENT_G_BIT : 0;
		BlendState.colorWriteMask |= (ColorTarget.ColorWriteMask & CW_BLUE) ? VK_COLOR_COMPONENT_B_BIT : 0;
		BlendState.colorWriteMask |= (ColorTarget.ColorWriteMask & CW_ALPHA) ? VK_COLOR_COMPONENT_A_BIT : 0;
    }
}

// FVulkanRenderTargetLayout::FVulkanRenderTargetLayout(const RHIRenderTargetLayout& RTLayout)
// {
//     InitWithAttachments(RTLayout.RenderTargetFormats, RTLayout.DepthStencilTargetFormat);
// }
//
// void FVulkanRenderTargetLayout::InitWithAttachments(        
//     const std::array<EPixelFormat, MaxSimultaneousRenderTargets>& RenderTargetFormats,
//     EPixelFormat DepthStencilTargetFormat)
// {
//     int32 NumColorAttachments = 0;
// 	for (uint32 Index = 0; Index < MaxSimultaneousRenderTargets; ++Index)
//     {
// 		EPixelFormat Format = RenderTargetFormats[Index];
// 		if (Format != EPixelFormat::PF_Unknown)
// 		{
// 			VkAttachmentDescription& CurrDesc = Desc.emplace_back();
// 			CurrDesc.samples = VK_SAMPLE_COUNT_1_BIT;
// 			CurrDesc.format = TranslatePixelFormatToVKFormat(Format);
// 			CurrDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
// 			CurrDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
// 			CurrDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
// 			CurrDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//
// 			// If the initial != final we need to change the FullHashInfo and use FinalLayout
// 			CurrDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
// 			CurrDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
// 			VkAttachmentReference& ColorRef = ColorReferences.emplace_back();
//             ColorRef.attachment = Index;
// 			ColorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//             Ncheck(CurrDesc.format != VK_FORMAT_UNDEFINED);
//             NumColorAttachments += 1;
// 		}
//     }
//     if (DepthStencilTargetFormat != EPixelFormat::PF_Unknown)
//     {
//         bHasDepthAttachment = true;
//         EPixelFormat Format = DepthStencilTargetFormat;
// 		VkAttachmentDescription& CurrDesc = Desc.emplace_back();
//         CurrDesc.samples = VK_SAMPLE_COUNT_1_BIT;
//         CurrDesc.format = TranslatePixelFormatToVKFormat(Format);
//         CurrDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
//         CurrDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//         CurrDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
//         CurrDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
//         CurrDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//         CurrDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//         DepthStencilReference.attachment = NumColorAttachments;
//         DepthStencilReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//         Ncheck(CurrDesc.format != VK_FORMAT_UNDEFINED);
//     }
// 
//     RenderPassFullHash = FCrc::MemCrc32(Desc.data(), sizeof(VkAttachmentDescription) * Desc.size());
// }

VkFramebuffer FVulkanRenderPassManager::GetOrCreateFramebuffer(VkRenderPass RenderPass, const std::array<RHITextureView*, MaxSimultaneousRenderTargets>& ColorAttachments, RHITextureView* DepthStencilAttachment)
{
    uint32 Width = 0, Height = 0;
    std::vector<VkImageView> Attachments;
    for (auto [Index, Texture] : Enumerate(ColorAttachments))
    {
        if (Texture)
        {
            VulkanTextureView* vkTexture = ResourceCast(Texture);
            Attachments.push_back(vkTexture->GetHandle());
            if (Width == 0 && Height == 0)
            {
                Width = Texture->GetSizeX();
                Height = Texture->GetSizeY();
            }
            else
            {
                Ncheck(Texture->GetSizeX() == Width);
                Ncheck(Texture->GetSizeY() == Height);
            }
        }
    }
    if (DepthStencilAttachment)
    {
        VulkanTextureView* vkTexture = ResourceCast(DepthStencilAttachment);
        Attachments.push_back(vkTexture->GetHandle());
        if (Width == 0 && Height == 0)
        {
            Width = DepthStencilAttachment->GetSizeX();
            Height = DepthStencilAttachment->GetSizeY();
        }
        else
        {
            Ncheck(DepthStencilAttachment->GetSizeX() == Width);
            Ncheck(DepthStencilAttachment->GetSizeY() == Height);
        }
    }

    Ncheck(Attachments.size() != 0);
    uint32 Hash = FCrc::MemCrc32(Attachments.data(), sizeof(VkImageView) * Attachments.size());
    auto Found = Framebuffers.find(Hash);
    if (Found != Framebuffers.end())
    {
        return Found->second;
    }

    VkFramebufferCreateInfo FramebufferInfo{};
    FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferInfo.width = Width;
    FramebufferInfo.height = Height;
    FramebufferInfo.renderPass = RenderPass;
    FramebufferInfo.attachmentCount = static_cast<uint32>(Attachments.size());
    FramebufferInfo.pAttachments = Attachments.data();
    FramebufferInfo.layers = 1;

    VkFramebuffer& Framebuffer = Framebuffers[Hash];
    VK_CHECK_RESULT(vkCreateFramebuffer(Device, &FramebufferInfo, nullptr, &Framebuffer));
    return Framebuffer;
}

VkRenderPass FVulkanRenderPassManager::GetOrCreateRenderPass(const RHIRenderTargetLayout& RTLayout)
{
    uint32 Hash = std::hash<RHIRenderTargetLayout>{}(RTLayout);
	auto Found = RenderPasses.find(Hash);
    if (Found != RenderPasses.end())
        return Found->second;

    // fill in the attachment descriptions and references
    std::vector<VkAttachmentDescription> AttachmentDescriptions;
    std::vector<VkAttachmentReference> ColorReferences;
    VkAttachmentReference DepthStencilReference{};
    {
        for (auto [Index, Attachment] : Enumerate(RTLayout.ColorAttachments))
        {
            EPixelFormat Format = Attachment.Format;
            if (Format != EPixelFormat::PF_Unknown)
            {
                VkAttachmentDescription CurrDesc{};
                CurrDesc.samples = VK_SAMPLE_COUNT_1_BIT;
                CurrDesc.format = TranslatePixelFormatToVKFormat(Format);
                CurrDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                CurrDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                CurrDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                CurrDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                CurrDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                CurrDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                AttachmentDescriptions.push_back(CurrDesc);

                VkAttachmentReference ColorRef{};
                ColorRef.attachment = Index;
                ColorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ColorReferences.push_back(ColorRef);
            }
        }
        EPixelFormat Format = RTLayout.DepthStencilAttachment.Format;
        if (Format != EPixelFormat::PF_Unknown)
        {
            VkAttachmentDescription CurrDesc{};
            CurrDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            CurrDesc.format = TranslatePixelFormatToVKFormat(Format);
            CurrDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            CurrDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            CurrDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            CurrDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            CurrDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            CurrDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            AttachmentDescriptions.push_back(CurrDesc);

            DepthStencilReference.attachment = AttachmentDescriptions.size() - 1;
            DepthStencilReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = ColorReferences.size();
    subpass.pColorAttachments = ColorReferences.data();
    if (DepthStencilReference.layout != VK_IMAGE_LAYOUT_UNDEFINED)
        subpass.pDepthStencilAttachment = &DepthStencilReference;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = AttachmentDescriptions.size();
    renderPassInfo.pAttachments = AttachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass& RenderPass = RenderPasses[Hash];
    VK_CHECK_RESULT(vkCreateRenderPass(Device, &renderPassInfo, nullptr, &RenderPass));
    return RenderPass;
}
FVulkanRenderPassManager::~FVulkanRenderPassManager()
{
    for (auto& [Hash, RenderPass] : RenderPasses)
    {
        vkDestroyRenderPass(Device, RenderPass, nullptr);
    }
    for (auto& [Hash, Framebuffer] : Framebuffers)
    {
        vkDestroyFramebuffer(Device, Framebuffer, nullptr);
    }
}

VkFormat TranslatePixelFormatToVKFormat(EPixelFormat Format)
{
    switch (Format) 
    {
    case EPixelFormat::PF_Unknown:
        return VK_FORMAT_UNDEFINED;
    case EPixelFormat::PF_R8:
        return VK_FORMAT_R8_UNORM;
    case EPixelFormat::PF_R8UI:
        return VK_FORMAT_R8_UINT;
    case EPixelFormat::PF_R8G8:
        return VK_FORMAT_R8G8_UNORM;
    case EPixelFormat::PF_R8G8B8:
        return VK_FORMAT_R8G8B8_UNORM;
    case EPixelFormat::PF_R8G8B8_sRGB:
        return VK_FORMAT_R8G8B8_SRGB;
    case EPixelFormat::PF_B8G8R8:
        return VK_FORMAT_B8G8R8_UNORM;
    case EPixelFormat::PF_B8G8R8_sRGB:
        return VK_FORMAT_B8G8R8_SRGB;
    case EPixelFormat::PF_R8G8B8A8:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case EPixelFormat::PF_R8G8B8A8_sRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case EPixelFormat::PF_B8G8R8A8:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case EPixelFormat::PF_B8G8R8A8_sRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case EPixelFormat::PF_D24S8:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case EPixelFormat::PF_D32F:
        return VK_FORMAT_D32_SFLOAT;
    case EPixelFormat::PF_D32FS8:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case EPixelFormat::PF_DXT1:
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case EPixelFormat::PF_DXT1_sRGB:
        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case EPixelFormat::PF_DXT5:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case EPixelFormat::PF_DXT5_sRGB:
        return VK_FORMAT_BC3_SRGB_BLOCK;
    case EPixelFormat::PF_R16F:
        return VK_FORMAT_R16_SFLOAT;
    case EPixelFormat::PF_R16G16F:
        return VK_FORMAT_R16G16_SFLOAT;
    case EPixelFormat::PF_R16G16B16F:
        return VK_FORMAT_R16G16B16_SFLOAT;
    case EPixelFormat::PF_R16G16B16A16F:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case EPixelFormat::PF_R32F:
        return VK_FORMAT_R32_SFLOAT;
    case EPixelFormat::PF_R32G32F:
        return VK_FORMAT_R32G32_SFLOAT;
    case EPixelFormat::PF_R32G32B32F:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case EPixelFormat::PF_R32G32B32A32F:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    default:
        throw "Unknown PixelFormat!";
    }
}

}

namespace std {

size_t hash<nilou::RHIRenderTargetLayout>::operator()(const nilou::RHIRenderTargetLayout& _Keyval) const noexcept
{
    return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

}