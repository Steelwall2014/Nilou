#pragma once
#include <string>
#include <vector>
#include <array>
#include "RHIDefinitions.h"
#include "Templates/EnumAsByte.h"
#include "SerializeHelper.h"
#include "Common/CoreUObject/Class.h"

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
		GENERATED_STRUCT_BODY()

		NPROPERTY()
		bool bEnableDepthWrite;

		NPROPERTY()
		ECompareFunction DepthTest;


		NPROPERTY()
		bool bEnableFrontFaceStencil;

		NPROPERTY()
		ECompareFunction FrontFaceStencilTest;

		NPROPERTY()
		EStencilOp FrontFaceStencilFailStencilOp;

		NPROPERTY()
		EStencilOp FrontFaceDepthFailStencilOp;

		NPROPERTY()
		EStencilOp FrontFacePassStencilOp;

		NPROPERTY()
		bool bEnableBackFaceStencil;

		NPROPERTY()
		ECompareFunction BackFaceStencilTest;

		NPROPERTY()
		EStencilOp BackFaceStencilFailStencilOp;

		NPROPERTY()
		EStencilOp BackFaceDepthFailStencilOp;

		NPROPERTY()
		EStencilOp BackFacePassStencilOp;

		NPROPERTY()
		uint8 StencilReadMask;

		NPROPERTY()
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

	// template<>
	// class TStaticSerializer<FDepthStencilStateInitializer>
	// {
	// public:
	// 	static void Serialize(const FDepthStencilStateInitializer &DepthStencilState, nlohmann::json &json, FArchiveBuffers &Buffers)
	// 	{
	// 		json["ClassName"] = "FDepthStencilStateInitializer";
	// 		nlohmann::json &content = json["Content"];
	// 		content["EnableDepthWrite"] = DepthStencilState.bEnableDepthWrite;
	// 		content["EnableFrontFaceStencil"] = DepthStencilState.bEnableFrontFaceStencil;
	// 		content["EnableBackFaceStencil"] = DepthStencilState.bEnableBackFaceStencil;
	// 		content["StencilReadMask"] = DepthStencilState.StencilReadMask;
	// 		content["StencilWriteMask"] = DepthStencilState.StencilWriteMask;
	// 		content["DepthTest"] = magic_enum::enum_name(DepthStencilState.DepthTest);
	// 		content["FrontFaceStencilTest"] = magic_enum::enum_name(DepthStencilState.FrontFaceStencilTest);
	// 		content["FrontFaceStencilFailStencilOp"] = magic_enum::enum_name(DepthStencilState.FrontFaceStencilFailStencilOp);
	// 		content["FrontFaceDepthFailStencilOp"] = magic_enum::enum_name(DepthStencilState.FrontFaceDepthFailStencilOp);
	// 		content["FrontFacePassStencilOp"] = magic_enum::enum_name(DepthStencilState.FrontFacePassStencilOp);
	// 		content["BackFaceStencilTest"] = magic_enum::enum_name(DepthStencilState.BackFaceStencilTest);
	// 		content["BackFaceStencilFailStencilOp"] = magic_enum::enum_name(DepthStencilState.BackFaceStencilFailStencilOp);
	// 		content["BackFaceDepthFailStencilOp"] = magic_enum::enum_name(DepthStencilState.BackFaceDepthFailStencilOp);
	// 		content["BackFacePassStencilOp"] = magic_enum::enum_name(DepthStencilState.BackFacePassStencilOp);
	// 	}
	// 	static void Deserialize(FDepthStencilStateInitializer &DepthStencilState, nlohmann::json &json, void* Buffer)
	// 	{
    //         if (!SerializeHelper::CheckIsType(json, "FDepthStencilStateInitializer")) return;
	// 		nlohmann::json &content = json["Content"];
	// 		DepthStencilState.bEnableDepthWrite = content["EnableDepthWrite"].get<bool>();
	// 		DepthStencilState.bEnableFrontFaceStencil = content["EnableFrontFaceStencil"].get<bool>();
	// 		DepthStencilState.bEnableBackFaceStencil = content["EnableBackFaceStencil"].get<bool>();
	// 		DepthStencilState.StencilReadMask = content["StencilReadMask"].get<uint8>();
	// 		DepthStencilState.StencilWriteMask = content["StencilWriteMask"].get<uint8>();
	// 		DepthStencilState.DepthTest = magic_enum::enum_cast<ECompareFunction>(content["DepthTest"].get<std::string>()).value();
	// 		DepthStencilState.FrontFaceStencilTest = magic_enum::enum_cast<ECompareFunction>(content["FrontFaceStencilTest"].get<std::string>()).value();
	// 		DepthStencilState.FrontFaceStencilFailStencilOp = magic_enum::enum_cast<EStencilOp>(content["FrontFaceStencilFailStencilOp"].get<std::string>()).value();
	// 		DepthStencilState.FrontFaceDepthFailStencilOp = magic_enum::enum_cast<EStencilOp>(content["FrontFaceDepthFailStencilOp"].get<std::string>()).value();
	// 		DepthStencilState.FrontFacePassStencilOp = magic_enum::enum_cast<EStencilOp>(content["FrontFacePassStencilOp"].get<std::string>()).value();
	// 		DepthStencilState.BackFaceStencilTest = magic_enum::enum_cast<ECompareFunction>(content["BackFaceStencilTest"].get<std::string>()).value();
	// 		DepthStencilState.BackFaceStencilFailStencilOp = magic_enum::enum_cast<EStencilOp>(content["BackFaceStencilFailStencilOp"].get<std::string>()).value();
	// 		DepthStencilState.BackFaceDepthFailStencilOp = magic_enum::enum_cast<EStencilOp>(content["BackFaceDepthFailStencilOp"].get<std::string>()).value();
	// 		DepthStencilState.BackFacePassStencilOp = magic_enum::enum_cast<EStencilOp>(content["BackFacePassStencilOp"].get<std::string>()).value();
	// 	}
	// };

    struct NSTRUCT FRasterizerStateInitializer
    {
		GENERATED_STRUCT_BODY()

		NPROPERTY()
        ERasterizerFillMode FillMode;

		NPROPERTY()
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

	// template<>
	// class TStaticSerializer<FRasterizerStateInitializer>
	// {
	// public:
	// 	static void Serialize(const FRasterizerStateInitializer &RasterizerState, nlohmann::json &json, FArchiveBuffers &Buffers)
	// 	{
	// 		json["ClassName"] = "FRasterizerStateInitializer";
	// 		nlohmann::json &content = json["Content"];
	// 		content["FillMode"] = magic_enum::enum_name(RasterizerState.FillMode);
	// 		content["CullMode"] = magic_enum::enum_name(RasterizerState.CullMode);
	// 	}
	// 	static void Deserialize(FRasterizerStateInitializer &RasterizerState, nlohmann::json &json, void* Buffer)
	// 	{
    //         if (!SerializeHelper::CheckIsType(json, "FRasterizerStateInitializer")) return;
	// 		nlohmann::json &content = json["Content"];
	// 		RasterizerState.CullMode = magic_enum::enum_cast<ERasterizerCullMode>(content["CullMode"].get<std::string>()).value();
	// 		RasterizerState.FillMode = magic_enum::enum_cast<ERasterizerFillMode>(content["FillMode"].get<std::string>()).value();
	// 	}
	// };

	struct NSTRUCT FBlendStateInitializer
	{
		GENERATED_STRUCT_BODY()

		struct NSTRUCT FRenderTarget
		{
			GENERATED_STRUCT_BODY()

			NPROPERTY()
			EBlendOperation ColorBlendOp;
			NPROPERTY()
			EBlendFactor ColorSrcBlend;
			NPROPERTY()
			EBlendFactor ColorDestBlend;
			NPROPERTY()
			EBlendOperation AlphaBlendOp;
			NPROPERTY()
			EBlendFactor AlphaSrcBlend;
			NPROPERTY()
			EBlendFactor AlphaDestBlend;
			NPROPERTY()
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
			static_assert(NumRenderTargets <= MaxSimultaneousRenderTargets, "Too many render target blend states.");

			for(uint32 RenderTargetIndex = 0;RenderTargetIndex < NumRenderTargets;++RenderTargetIndex)
			{
				RenderTargets[RenderTargetIndex] = InRenderTargetBlendStates[RenderTargetIndex];
			}
		}

		NPROPERTY()
		std::array<FRenderTarget, MaxSimultaneousRenderTargets> RenderTargets;
		
		NPROPERTY()
		bool bUseIndependentRenderTargetBlendStates;
		// bool bUseAlphaToCoverage;
	};

	// template<>
	// class TStaticSerializer<FBlendStateInitializer>
	// {
	// public:
	// 	static void Serialize(const FBlendStateInitializer &BlendState, nlohmann::json &json, FArchiveBuffers &Buffers)
	// 	{
	// 		json["ClassName"] = "FBlendStateInitializer";
	// 		nlohmann::json &content = json["Content"];
	// 		content["UseIndependentRenderTargetBlendStates"] = BlendState.bUseIndependentRenderTargetBlendStates;
	// 		nlohmann::json &render_targets = content["RenderTargets"];
	// 		for (int i = 0; i < MaxSimultaneousRenderTargets; i++)
	// 		{
	// 			nlohmann::json render_target;
	// 			render_target["ColorBlendOp"] = magic_enum::enum_name(BlendState.RenderTargets[i].ColorBlendOp);
	// 			render_target["AlphaBlendOp"] = magic_enum::enum_name(BlendState.RenderTargets[i].AlphaBlendOp);
	// 			render_target["AlphaDestBlend"] = magic_enum::enum_name(BlendState.RenderTargets[i].AlphaDestBlend);
	// 			render_target["AlphaSrcBlend"] = magic_enum::enum_name(BlendState.RenderTargets[i].AlphaSrcBlend);
	// 			render_target["ColorDestBlend"] = magic_enum::enum_name(BlendState.RenderTargets[i].ColorDestBlend);
	// 			render_target["ColorSrcBlend"] = magic_enum::enum_name(BlendState.RenderTargets[i].ColorSrcBlend);
	// 			render_target["ColorWriteMask"] = magic_enum::enum_name(BlendState.RenderTargets[i].ColorWriteMask);
	// 			render_targets.push_back(render_target);
	// 		}
	// 	}
	// 	static void Deserialize(FBlendStateInitializer &BlendState, nlohmann::json &json, void* Buffer)
	// 	{
    //         if (!SerializeHelper::CheckIsType(json, "FBlendStateInitializer")) return;

	// 		nlohmann::json content = json["Content"];
	// 		BlendState.bUseIndependentRenderTargetBlendStates = content["UseIndependentRenderTargetBlendStates"];

	// 		nlohmann::json render_targets = content["RenderTargets"];
	// 		for (int i = 0; i < MaxSimultaneousRenderTargets; i++)
	// 		{
	// 			nlohmann::json render_target = render_targets[i];
	// 			BlendState.RenderTargets[i].ColorBlendOp = magic_enum::enum_cast<EBlendOperation>(render_target["ColorBlendOp"].get<std::string>()).value();
	// 			BlendState.RenderTargets[i].AlphaBlendOp = magic_enum::enum_cast<EBlendOperation>(render_target["AlphaBlendOp"].get<std::string>()).value();
	// 			BlendState.RenderTargets[i].AlphaDestBlend = magic_enum::enum_cast<EBlendFactor>(render_target["AlphaDestBlend"].get<std::string>()).value();
	// 			BlendState.RenderTargets[i].AlphaSrcBlend = magic_enum::enum_cast<EBlendFactor>(render_target["AlphaSrcBlend"].get<std::string>()).value();
	// 			BlendState.RenderTargets[i].ColorDestBlend = magic_enum::enum_cast<EBlendFactor>(render_target["ColorDestBlend"].get<std::string>()).value();
	// 			BlendState.RenderTargets[i].ColorSrcBlend = magic_enum::enum_cast<EBlendFactor>(render_target["ColorSrcBlend"].get<std::string>()).value();
	// 			BlendState.RenderTargets[i].ColorWriteMask = magic_enum::enum_cast<EColorWriteMask>(render_target["ColorWriteMask"].get<std::string>()).value();
	// 		}
	// 	}
	// };

	struct NSTRUCT FSamplerStateInitializer
	{
		GENERATED_STRUCT_BODY()

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