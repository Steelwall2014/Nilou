#include "VulkanResources.h"

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

VulkanDepthStencilState::VulkanDepthStencilState(const FDepthStencilStateInitializer& Initializer)
{
    DepthStencilState = VkPipelineDepthStencilStateCreateInfo{};
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
    RasterizerState = VkPipelineRasterizationStateCreateInfo{};
	RasterizerState.polygonMode = RasterizerFillModeToVulkan(Initializer.FillMode);
	RasterizerState.cullMode = RasterizerCullModeToVulkan(Initializer.CullMode);

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

}