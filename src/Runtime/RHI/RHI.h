#pragma once
#include <string>
#include <vector>
#include <array>
#include "RHIDefinitions.h"
#include "Templates/EnumAsByte.h"

namespace nilou {
        
    struct FVertexElement
    {
        uint8 Offset;
        EVertexElementType Type;
        uint8 AttributeIndex;
        uint16 Stride;

        FVertexElement() {}
        FVertexElement(uint8 InOffset,EVertexElementType InType,uint8 InAttributeIndex,uint16 InStride):
            Offset(InOffset),
            Type(InType),
            AttributeIndex(InAttributeIndex),
            Stride(InStride)
        {}

        void operator=(const FVertexElement& Other)
        {
            Offset = Other.Offset;
            Type = Other.Type;
            AttributeIndex = Other.AttributeIndex;
            Stride = Other.Stride;
        }
    };

    using FVertexDeclarationElementList = std::vector<FVertexElement>;


	struct FDepthStencilStateInitializer
	{
		bool bEnableDepthWrite;
		ECompareFunction DepthTest;

		bool bEnableFrontFaceStencil;
		ECompareFunction FrontFaceStencilTest;
		EStencilOp FrontFaceStencilFailStencilOp;
		EStencilOp FrontFaceDepthFailStencilOp;
		EStencilOp FrontFacePassStencilOp;
		bool bEnableBackFaceStencil;
		ECompareFunction BackFaceStencilTest;
		EStencilOp BackFaceStencilFailStencilOp;
		EStencilOp BackFaceDepthFailStencilOp;
		EStencilOp BackFacePassStencilOp;
		uint8 StencilReadMask;
		uint8 StencilWriteMask;

		FDepthStencilStateInitializer(
			bool bInEnableDepthWrite = true,
			ECompareFunction InDepthTest = CF_Less,
			bool bInEnableFrontFaceStencil = false,
			ECompareFunction InFrontFaceStencilTest = CF_Always,
			EStencilOp InFrontFaceStencilFailStencilOp = EStencilOp::SO_Keep,
			EStencilOp InFrontFaceDepthFailStencilOp = EStencilOp::SO_Keep,
			EStencilOp InFrontFacePassStencilOp = EStencilOp::SO_Keep,
			bool bInEnableBackFaceStencil = false,
			ECompareFunction InBackFaceStencilTest = CF_Always,
			EStencilOp InBackFaceStencilFailStencilOp = EStencilOp::SO_Keep,
			EStencilOp InBackFaceDepthFailStencilOp = EStencilOp::SO_Keep,
			EStencilOp InBackFacePassStencilOp = EStencilOp::SO_Keep,
			uint8 InStencilReadMask = 0xFF,
			uint8 InStencilWriteMask = 0xFF
			)
		: bEnableDepthWrite(bInEnableDepthWrite)
		, DepthTest(InDepthTest)
		, bEnableFrontFaceStencil(bInEnableFrontFaceStencil)
		, FrontFaceStencilTest(InFrontFaceStencilTest)
		, FrontFaceStencilFailStencilOp(InFrontFaceStencilFailStencilOp)
		, FrontFaceDepthFailStencilOp(InFrontFaceDepthFailStencilOp)
		, FrontFacePassStencilOp(InFrontFacePassStencilOp)
		, bEnableBackFaceStencil(bInEnableBackFaceStencil)
		, BackFaceStencilTest(InBackFaceStencilTest)
		, BackFaceStencilFailStencilOp(InBackFaceStencilFailStencilOp)
		, BackFaceDepthFailStencilOp(InBackFaceDepthFailStencilOp)
		, BackFacePassStencilOp(InBackFacePassStencilOp)
		, StencilReadMask(InStencilReadMask)
		, StencilWriteMask(InStencilWriteMask)
		{}

		std::string ToString() const
		{
			char buffer[256];
			std::string res;
			std::sprintf("<%u %u "
				, buffer
				, uint32(!!bEnableDepthWrite)
				, uint32(DepthTest)
			);
			res += buffer;
			std::sprintf("%u %u %u %u %u "
				, buffer
				, uint32(!!bEnableFrontFaceStencil)
				, uint32(FrontFaceStencilTest)
				, uint32(FrontFaceStencilFailStencilOp)
				, uint32(FrontFaceDepthFailStencilOp)
				, uint32(FrontFacePassStencilOp)
			);
			res += buffer;
			std::sprintf("%u %u %u %u %u "
				, buffer
				, uint32(!!bEnableBackFaceStencil)
				, uint32(BackFaceStencilTest)
				, uint32(BackFaceStencilFailStencilOp)
				, uint32(BackFaceDepthFailStencilOp)
				, uint32(BackFacePassStencilOp)
			);
			res += buffer;
			std::sprintf("%u %u>"
				, buffer
				, uint32(StencilReadMask)
				, uint32(StencilWriteMask)
			);
			return res;
		}
	};

    struct FRasterizerStateInitializer
    {
        ERasterizerFillMode FillMode;
        ERasterizerCullMode CullMode;
        // float DepthBias;
        // float SlopeScaleDepthBias;
        // bool bAllowMSAA;
        // bool bEnableLineAA;

		FRasterizerStateInitializer(
			ERasterizerFillMode InFillMode = FM_Solid,
			ERasterizerCullMode InCullMode = CM_CW)
			: FillMode(InFillMode)
			, CullMode(InCullMode)
		{

		}
        
        inline bool operator== (const FRasterizerStateInitializer& B)
		{
			return this->FillMode == B.FillMode && 
				   this->CullMode == B.CullMode/* &&
				   this->bAllowMSAA == B.bAllowMSAA &&
				   this->bEnableLineAA == B.bEnableLineAA*/;
		}
    };

	class FBlendStateInitializer
	{
	public:

		struct FRenderTarget
		{
			EBlendOperation ColorBlendOp;
			EBlendFactor ColorSrcBlend;
			EBlendFactor ColorDestBlend;
			EBlendOperation AlphaBlendOp;
			EBlendFactor AlphaSrcBlend;
			EBlendFactor AlphaDestBlend;
			EColorWriteMask ColorWriteMask;
			
			FRenderTarget(
				EBlendOperation InColorBlendOp = BO_Add,
				EBlendFactor InColorSrcBlend = BF_One,
				EBlendFactor InColorDestBlend = BF_Zero,
				EBlendOperation InAlphaBlendOp = BO_Add,
				EBlendFactor InAlphaSrcBlend = BF_One,
				EBlendFactor InAlphaDestBlend = BF_Zero,
				EColorWriteMask InColorWriteMask = CW_RGBA
				)
			: ColorBlendOp(InColorBlendOp)
			, ColorSrcBlend(InColorSrcBlend)
			, ColorDestBlend(InColorDestBlend)
			, AlphaBlendOp(InAlphaBlendOp)
			, AlphaSrcBlend(InAlphaSrcBlend)
			, AlphaDestBlend(InAlphaDestBlend)
			, ColorWriteMask(InColorWriteMask)
			{}
		};

		FBlendStateInitializer() 
		:	bUseIndependentRenderTargetBlendStates(false)
		{}

		FBlendStateInitializer(const FRenderTarget& InRenderTargetBlendState/*, bool bInUseAlphaToCoverage = false*/)
		:	bUseIndependentRenderTargetBlendStates(false)
		// ,	bUseAlphaToCoverage(bInUseAlphaToCoverage)
		{
			RenderTargets[0] = InRenderTargetBlendState;
		}

		template<uint64 NumRenderTargets>
		FBlendStateInitializer(const std::array<FRenderTarget, NumRenderTargets>& InRenderTargetBlendStates/*, bool bInUseAlphaToCoverage = false*/)
		:	bUseIndependentRenderTargetBlendStates(NumRenderTargets > 1)
		// ,	bUseAlphaToCoverage(bInUseAlphaToCoverage)
		{
			static_assert(NumRenderTargets <= MAX_SIMULTANEOUS_RENDERTARGETS, "Too many render target blend states.");

			for(uint32 RenderTargetIndex = 0;RenderTargetIndex < NumRenderTargets;++RenderTargetIndex)
			{
				RenderTargets[RenderTargetIndex] = InRenderTargetBlendStates[RenderTargetIndex];
			}
		}

		std::array<FRenderTarget, MAX_SIMULTANEOUS_RENDERTARGETS> RenderTargets;
		bool bUseIndependentRenderTargetBlendStates;
		// bool bUseAlphaToCoverage;
	};

}