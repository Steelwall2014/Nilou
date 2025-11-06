#pragma once
#include <string>
#include <vector>
#include <array>
#include "RHIDefinitions.h"
#include "Templates/EnumAsByte.h"

namespace nilou {
        
    struct FVertexElement
    {
		uint8 StreamIndex;
        uint8 Offset;
        EVertexElementType Type;
        uint8 AttributeIndex;
        uint16 Stride;

        FVertexElement() :
            StreamIndex(0),
            Offset(0),
            Type(EVertexElementType::None),
            AttributeIndex(0),
            Stride(0)
		{}
        FVertexElement(uint8 InStreamIndex,uint8 InOffset,EVertexElementType InType,uint8 InAttributeIndex,uint16 InStride):
			StreamIndex(InStreamIndex),
            Offset(InOffset),
            Type(InType),
            AttributeIndex(InAttributeIndex),
            Stride(InStride)
        {}
    };

    using FVertexDeclarationElementList = std::array<FVertexElement, MAX_VERTEX_ELEMENTS>;

    class FVertexStreamComponent
    {
    public:
        class FVertexBuffer *VertexBuffer;
        uint8 Offset;
        uint8 Stride;
        EVertexElementType Type;

        FVertexStreamComponent() 
            : VertexBuffer(nullptr)
            , Offset(0)
            , Stride(0)
            , Type(EVertexElementType::None)
        { }

        FVertexStreamComponent(FVertexBuffer *InVertexBuffer, uint8 InOffset, uint8 InStride, EVertexElementType InType) 
            : VertexBuffer(InVertexBuffer)
            , Offset(InOffset)
            , Stride(InStride)
            , Type(InType)
        { }
    };
    using FVertexStreamComponentList = std::vector<FVertexStreamComponent *>;

    struct FVertexInputStream
    {
        uint32 StreamIndex : 4;
        uint32 Offset : 28;
        class RDGBuffer* VertexBuffer;

        FVertexInputStream() :
            StreamIndex(0),
            Offset(0),
            VertexBuffer(nullptr)
        {}

        FVertexInputStream(uint32 InStreamIndex, uint32 InOffset, RDGBuffer* InVertexBuffer)
            : StreamIndex(InStreamIndex), Offset(InOffset), VertexBuffer(InVertexBuffer)
        {
        }

        inline bool operator==(const FVertexInputStream& rhs) const
        {
            if (StreamIndex != rhs.StreamIndex ||
                Offset != rhs.Offset || 
                VertexBuffer != rhs.VertexBuffer) 
            {
                return false;
            }

            return true;
        }

        inline bool operator!=(const FVertexInputStream& rhs) const
        {
            return !(*this == rhs);
        }
    };


	struct NSTRUCT FDepthStencilStateInitializer
	{
		GENERATED_BODY()

		NPROPERTY()
		bool bEnableDepthWrite = true;

		NPROPERTY()
		ECompareFunction DepthTest = CF_Less;


		NPROPERTY()
		bool bEnableFrontFaceStencil = false;

		NPROPERTY()
		ECompareFunction FrontFaceStencilTest = CF_Always;

		NPROPERTY()
		EStencilOp FrontFaceStencilFailStencilOp = EStencilOp::SO_Keep;

		NPROPERTY()
		EStencilOp FrontFaceDepthFailStencilOp = EStencilOp::SO_Keep;

		NPROPERTY()
		EStencilOp FrontFacePassStencilOp = EStencilOp::SO_Keep;

		NPROPERTY()
		bool bEnableBackFaceStencil = false;

		NPROPERTY()
		ECompareFunction BackFaceStencilTest = CF_Always;

		NPROPERTY()
		EStencilOp BackFaceStencilFailStencilOp = EStencilOp::SO_Keep;

		NPROPERTY()
		EStencilOp BackFaceDepthFailStencilOp = EStencilOp::SO_Keep;

		NPROPERTY()
		EStencilOp BackFacePassStencilOp = EStencilOp::SO_Keep;

		NPROPERTY()
		int32 StencilReadMask = 0xFF;

		NPROPERTY()
		int32 StencilWriteMask = 0xFF;

		// std::string ToString() const
		// {
		// 	char buffer[256];
		// 	std::string res;
		// 	std::sprintf("<%u %u "
		// 		, buffer
		// 		, uint32(!!bEnableDepthWrite)
		// 		, uint32(DepthTest)
		// 	);
		// 	res += buffer;
		// 	std::sprintf("%u %u %u %u %u "
		// 		, buffer
		// 		, uint32(!!bEnableFrontFaceStencil)
		// 		, uint32(FrontFaceStencilTest)
		// 		, uint32(FrontFaceStencilFailStencilOp)
		// 		, uint32(FrontFaceDepthFailStencilOp)
		// 		, uint32(FrontFacePassStencilOp)
		// 	);
		// 	res += buffer;
		// 	std::sprintf("%u %u %u %u %u "
		// 		, buffer
		// 		, uint32(!!bEnableBackFaceStencil)
		// 		, uint32(BackFaceStencilTest)
		// 		, uint32(BackFaceStencilFailStencilOp)
		// 		, uint32(BackFaceDepthFailStencilOp)
		// 		, uint32(BackFacePassStencilOp)
		// 	);
		// 	res += buffer;
		// 	std::sprintf("%u %u>"
		// 		, buffer
		// 		, uint32(StencilReadMask)
		// 		, uint32(StencilWriteMask)
		// 	);
		// 	return res;
		// }
	};

    struct NSTRUCT FRasterizerStateInitializer
    {
		GENERATED_BODY()

		NPROPERTY()
        ERasterizerFillMode FillMode = FM_Solid;

		NPROPERTY()
        ERasterizerCullMode CullMode = CM_CW;
        // float DepthBias;
        // float SlopeScaleDepthBias;
        // bool bAllowMSAA;
        // bool bEnableLineAA;
    };

	struct NSTRUCT FRenderTargetBlendState
	{
		GENERATED_BODY()

		NPROPERTY()
		EBlendOperation ColorBlendOp = BO_Add;
		NPROPERTY()
		EBlendFactor ColorSrcBlend = BF_One;
		NPROPERTY()
		EBlendFactor ColorDestBlend = BF_Zero;
		NPROPERTY()
		EBlendOperation AlphaBlendOp = BO_Add;
		NPROPERTY()
		EBlendFactor AlphaSrcBlend = BF_One;
		NPROPERTY()
		EBlendFactor AlphaDestBlend = BF_Zero;
		NPROPERTY()
		EColorWriteMask ColorWriteMask = CW_RGBA;
		
	};

	struct NSTRUCT FBlendStateInitializer
	{
		GENERATED_BODY()

		NPROPERTY()
		TArray<FRenderTargetBlendState> RenderTargets;
		
		NPROPERTY()
		bool bUseIndependentRenderTargetBlendStates = false;

		// NPROPERTY()
		// bool bUseAlphaToCoverage = false;

		FBlendStateInitializer() 
		:	bUseIndependentRenderTargetBlendStates(false)
		{
			RenderTargets.SetNum(MaxSimultaneousRenderTargets);
		}

		FBlendStateInitializer(const FRenderTargetBlendState& InRenderTargetBlendState/*, bool bInUseAlphaToCoverage = false*/)
		:	bUseIndependentRenderTargetBlendStates(false)
		// ,	bUseAlphaToCoverage(bInUseAlphaToCoverage)
		{
			RenderTargets.SetNum(MaxSimultaneousRenderTargets);
			RenderTargets[0] = InRenderTargetBlendState;
		}

		template<uint64 NumRenderTargets>
		FBlendStateInitializer(const std::array<FRenderTargetBlendState, NumRenderTargets>& InRenderTargetBlendStates/*, bool bInUseAlphaToCoverage = false*/)
		:	bUseIndependentRenderTargetBlendStates(NumRenderTargets > 1)
		// ,	bUseAlphaToCoverage(bInUseAlphaToCoverage)
		{
			static_assert(NumRenderTargets <= MaxSimultaneousRenderTargets, "Too many render target blend states.");

			RenderTargets.SetNum(MaxSimultaneousRenderTargets);
			for(uint32 RenderTargetIndex = 0;RenderTargetIndex < NumRenderTargets;++RenderTargetIndex)
			{
				RenderTargets[RenderTargetIndex] = InRenderTargetBlendStates[RenderTargetIndex];
			}
		}
	};

	struct NSTRUCT FSamplerStateInitializer
	{
		GENERATED_BODY()

		FSamplerStateInitializer() {}
		FSamplerStateInitializer(
			ESamplerFilter InFilter,
			ESamplerAddressMode InAddressU = AM_Wrap,
			ESamplerAddressMode InAddressV = AM_Wrap,
			ESamplerAddressMode InAddressW = AM_Wrap,
			float InMipBias = 0,
			int32 InMaxAnisotropy = 0,
			float InMinMipLevel = 0,
			float InMaxMipLevel = FLT_MAX,
			uint32 InBorderColor = 0,
			/** Only supported in D3D11 */
			ESamplerCompareFunction InSamplerComparisonFunction = SCF_Never
			)
		:	Filter(InFilter)
		,	AddressU(InAddressU)
		,	AddressV(InAddressV)
		,	AddressW(InAddressW)
		,	MipBias(InMipBias)
		,	MinMipLevel(InMinMipLevel)
		,	MaxMipLevel(InMaxMipLevel)
		,	MaxAnisotropy(InMaxAnisotropy)
		,	BorderColor(InBorderColor)
		,	SamplerComparisonFunction(InSamplerComparisonFunction)
		{
		}

		NPROPERTY()
		ESamplerFilter Filter = SF_Point;

		NPROPERTY()
		ESamplerAddressMode AddressU = AM_Wrap;

		NPROPERTY()
		ESamplerAddressMode AddressV = AM_Wrap;

		NPROPERTY()
		ESamplerAddressMode AddressW = AM_Wrap;

		NPROPERTY()
		float MipBias = 0.0f;

		/** Smallest mip map level that will be used, where 0 is the highest resolution mip level. */
		NPROPERTY()
		float MinMipLevel = 0.0f;

		/** Largest mip map level that will be used, where 0 is the highest resolution mip level. */
		NPROPERTY()
		float MaxMipLevel = FLT_MAX;
		
		NPROPERTY()
		int32 MaxAnisotropy = 0;
		
		NPROPERTY()
		uint32 BorderColor = 0;

		NPROPERTY()
		ESamplerCompareFunction SamplerComparisonFunction = SCF_Never;
	};

}