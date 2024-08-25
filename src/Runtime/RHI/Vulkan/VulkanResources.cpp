#include "Common/Crc.h"
#include "VulkanDynamicRHI.h"

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
	vkDestroyPipeline(Device, VulkanPipeline, nullptr);
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
    for (int i = 0; i < MAX_SIMULTANEOUS_RENDERTARGETS; i++)
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

FVulkanRenderTargetLayout::FVulkanRenderTargetLayout(const std::unordered_map<EFramebufferAttachment, EPixelFormat>& Attachments)
{
    std::array<EPixelFormat, MAX_SIMULTANEOUS_RENDERTARGETS> RenderTargetFormats;
    uint32 NumRenderTargetsEnabled = 0;
    EPixelFormat DepthStencilTargetFormat;
    for (auto [Attachment, Format] : Attachments)
    {
        if (Attachment == EFramebufferAttachment::FA_Depth_Stencil_Attachment)
        {
            DepthStencilTargetFormat = Format;
        }
        else 
        {
            uint32 index = (uint8)Attachment-(uint8)EFramebufferAttachment::FA_Color_Attachment0;
            RenderTargetFormats[index] = Format;
            NumRenderTargetsEnabled = std::max(NumRenderTargetsEnabled, index+1);
        }
    }
    InitWithAttachments(RenderTargetFormats, NumRenderTargetsEnabled, DepthStencilTargetFormat);
}

FVulkanRenderTargetLayout::FVulkanRenderTargetLayout(const FGraphicsPipelineStateInitializer& Initializer)
{
    InitWithAttachments(Initializer.RenderTargetFormats, Initializer.NumRenderTargetsEnabled, Initializer.DepthStencilTargetFormat);
}

FVulkanRenderTargetLayout::FVulkanRenderTargetLayout(const FRHIRenderPassInfo& Info)
{
    std::array<EPixelFormat, MAX_SIMULTANEOUS_RENDERTARGETS> RenderTargetFormats;
    RenderTargetFormats.fill(PF_Unknown);
    uint32 NumRenderTargetsEnabled = 0;
    EPixelFormat DepthStencilTargetFormat = PF_Unknown;
    for (auto [Attachment, Texture] : Info.Framebuffer->Attachments)
    {
        if (Attachment == EFramebufferAttachment::FA_Depth_Stencil_Attachment)
        {
            DepthStencilTargetFormat = Texture->GetFormat();
        }
        else 
        {
            uint32 index = (uint8)Attachment-(uint8)EFramebufferAttachment::FA_Color_Attachment0;
            RenderTargetFormats[index] = Texture->GetFormat();
            NumRenderTargetsEnabled = std::max(NumRenderTargetsEnabled, index+1);
        }
    }
    InitWithAttachments(RenderTargetFormats, NumRenderTargetsEnabled, DepthStencilTargetFormat);
    if (Info.bClearDepthBuffer)
    {
        Ncheck(DepthStencilReference.attachment < Desc.size());
        VkAttachmentDescription& CurrDesc = Desc[DepthStencilReference.attachment];
        CurrDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    }
    if (Info.bClearStencilBuffer)
    {
        Ncheck(DepthStencilReference.attachment < Desc.size());
        VkAttachmentDescription& CurrDesc = Desc[DepthStencilReference.attachment];
        CurrDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    }
    if (Info.bClearColorBuffer)
    {
        Ncheck(DepthStencilReference.attachment < Desc.size());
        for (auto& Ref : ColorReferences)
        {
            VkAttachmentDescription& CurrDesc = Desc[Ref.attachment];
            CurrDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        }
    }
}

void FVulkanRenderTargetLayout::InitWithAttachments(        
    const std::array<EPixelFormat, MAX_SIMULTANEOUS_RENDERTARGETS>& RenderTargetFormats,
    uint32 NumRenderTargetsEnabled,
    EPixelFormat DepthStencilTargetFormat)
{
	for (uint32 Index = 0; Index < NumRenderTargetsEnabled; ++Index)
    {
		EPixelFormat Format = RenderTargetFormats[Index];
		if (Format != EPixelFormat::PF_Unknown)
		{
			VkAttachmentDescription& CurrDesc = Desc.emplace_back();
			CurrDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			CurrDesc.format = TranslatePixelFormatToVKFormat(Format);
			CurrDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			CurrDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			CurrDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			CurrDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			// If the initial != final we need to change the FullHashInfo and use FinalLayout
			CurrDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			CurrDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference& ColorRef = ColorReferences.emplace_back();
            ColorRef.attachment = Index;
			ColorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            Ncheck(CurrDesc.format != VK_FORMAT_UNDEFINED);
		}
    }
    if (DepthStencilTargetFormat != EPixelFormat::PF_Unknown)
    {
        bHasDepthAttachment = true;
        EPixelFormat Format = DepthStencilTargetFormat;
		VkAttachmentDescription& CurrDesc = Desc.emplace_back();
        CurrDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        CurrDesc.format = TranslatePixelFormatToVKFormat(Format);
        CurrDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        CurrDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        CurrDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        CurrDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        CurrDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        CurrDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        DepthStencilReference.attachment = NumRenderTargetsEnabled;
        DepthStencilReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        Ncheck(CurrDesc.format != VK_FORMAT_UNDEFINED);
    }
    
    RenderPassFullHash = FCrc::MemCrc32(Desc.data(), sizeof(VkAttachmentDescription) * Desc.size());
}

FVulkanRenderPass* FVulkanRenderPassManager::GetOrCreateRenderPass(const FVulkanRenderTargetLayout& RTLayout)
{
	auto Found = RenderPasses.find(RTLayout);
    if (Found != RenderPasses.end())
        return &Found->second;

    FVulkanRenderPass& RenderPass = RenderPasses.emplace(RTLayout, Device).first->second;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = RTLayout.ColorReferences.size();
    subpass.pColorAttachments = RTLayout.ColorReferences.data();
    if (RTLayout.bHasDepthAttachment)
        subpass.pDepthStencilAttachment = &RTLayout.DepthStencilReference;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    Ncheck(RTLayout.Desc.size() != 0);
    if (RTLayout.Desc.size() == 0)
        throw "";
    renderPassInfo.attachmentCount = static_cast<uint32>(RTLayout.Desc.size());
    renderPassInfo.pAttachments = RTLayout.Desc.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(Device, &renderPassInfo, nullptr, &RenderPass.Handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    return &RenderPass;
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