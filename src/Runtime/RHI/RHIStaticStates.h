#include "RenderResource.h"
#include "DynamicRHI.h"

namespace nilou {
	template<typename InitializerType,typename RHIRefType,typename RHIParamRefType>
	class TStaticStateRHI
	{
	public:
		static RHIParamRefType GetRHI()
		{
			return StaticResource.StateRHI;
		};

	private:
		/** A resource which manages the RHI resource. */
		class FStaticStateResource : public FRenderResource
		{
		public:
			RHIRefType StateRHI;
			FStaticStateResource()
			{
				BeginInitResource(this);
			}

			// FRenderResource interface.
			virtual void InitRHI() override
			{
				StateRHI = InitializerType::CreateRHI();
			}
			virtual void ReleaseRHI() override final
			{
				StateRHI.SafeRelease();
			}
			virtual void ReleaseResource() override final
			{
				FRenderResource::ReleaseResource();
			}

			~FStaticStateResource()
			{
				ReleaseResource();
			}
		};

		static FStaticStateResource StaticResource;
	};

	
	/*	bool bEnableDepthWrite;
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
		uint8 StencilWriteMask;*/
	template<
		bool bEnableDepthWrite = true,
		ECompareFunction DepthTest = CF_Less,
		bool bEnableFrontFaceStencil = false,
		ECompareFunction FrontFaceStencilTest = CF_Always,
		EStencilOp FrontFaceStencilFailStencilOp = EStencilOp::SO_Keep,
		EStencilOp FrontFaceDepthFailStencilOp = EStencilOp::SO_Keep,
		EStencilOp FrontFacePassStencilOp = EStencilOp::SO_Keep,
		bool bEnableBackFaceStencil = false,
		ECompareFunction BackFaceStencilTest = CF_Always,
		EStencilOp BackFaceStencilFailStencilOp = EStencilOp::SO_Keep,
		EStencilOp BackFaceDepthFailStencilOp = EStencilOp::SO_Keep,
		EStencilOp BackFacePassStencilOp = EStencilOp::SO_Keep,
		uint8 StencilReadMask = 0xFF,
		uint8 StencilWriteMask = 0xFF
		>
	class TStaticDepthStencilState : public TStaticStateRHI<
        TStaticDepthStencilState<
			bEnableDepthWrite,
			DepthTest,
			bEnableFrontFaceStencil,
			FrontFaceStencilTest,
			FrontFaceStencilFailStencilOp,
			FrontFaceDepthFailStencilOp,
			FrontFacePassStencilOp,
			bEnableBackFaceStencil,
			BackFaceStencilTest,
			BackFaceStencilFailStencilOp,
			BackFaceDepthFailStencilOp,
			BackFacePassStencilOp,
			StencilReadMask,
			StencilWriteMask
			>, 
        RHIDepthStencilStateRef, 
        RHIDepthStencilState *>
	{
	public:
		static RHIDepthStencilStateRef CreateRHI()
		{
			FDepthStencilStateInitializer Initializer(
				bEnableDepthWrite,
				DepthTest,
				bEnableFrontFaceStencil,
				FrontFaceStencilTest,
				FrontFaceStencilFailStencilOp,
				FrontFaceDepthFailStencilOp,
				FrontFacePassStencilOp,
				bEnableBackFaceStencil,
				BackFaceStencilTest,
				BackFaceStencilFailStencilOp,
				BackFaceDepthFailStencilOp,
				BackFacePassStencilOp,
				StencilReadMask,
				StencilWriteMask);

			static RHIDepthStencilStateRef RHI = GDynamicRHI->RHICreateDepthStencilState(Initializer);
			return RHI;
		}
	};

	
	template<
		ERasterizerFillMode FillMode = FM_Solid,
        ERasterizerCullMode CullMode = CM_CW/*,
		bool bAllowMSAA = false,
		bool bEnableLineAA = false*/
		>
	class TStaticRasterizerState : public TStaticStateRHI<
        TStaticRasterizerState<
			FillMode,
			CullMode/*,
			bAllowMSAA,
			bEnableLineAA*/
			>, 
        RHIRasterizerStateRef, 
        RHIRasterizerState *>
	{
	public:
		static RHIRasterizerStateRef CreateRHI()
		{
			FRasterizerStateInitializer Initializer(
				FillMode,
				CullMode
			);

			static RHIRasterizerStateRef RHI = GDynamicRHI->RHICreateRasterizerState(Initializer);
			return RHI;
		}
	};


	/**
	* A static RHI blend state resource.
	* TStaticBlendStateRHI<...>::GetStaticState() will return a FBlendStateRHIRef to a blend state with the desired settings.
	* Should only be used from the rendering thread.
	* 
	* Alpha blending happens on GPU's as:
	* FinalColor.rgb = SourceColor * ColorSrcBlend (ColorBlendOp) DestColor * ColorDestBlend;
	* if (BlendState->bSeparateAlphaBlendEnable)
	*		FinalColor.a = SourceAlpha * AlphaSrcBlend (AlphaBlendOp) DestAlpha * AlphaDestBlend;
	* else
	*		Alpha blended the same way as rgb
	* 
	* Where source is the color coming from the pixel shader, and target is the color in the render target.
	*
	* So for example, TStaticBlendState<BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_Zero,BF_One> produces:
	* FinalColor.rgb = SourceColor * SourceAlpha + DestColor * (1 - SourceAlpha);
	* FinalColor.a = SourceAlpha * 0 + DestAlpha * 1;
	*/
	template<
		EColorWriteMask RT0ColorWriteMask = CW_RGBA,
		EBlendOperation RT0ColorBlendOp = BO_Add,
		EBlendFactor    RT0ColorSrcBlend = BF_One,
		EBlendFactor    RT0ColorDestBlend = BF_Zero,
		EBlendOperation RT0AlphaBlendOp = BO_Add,
		EBlendFactor    RT0AlphaSrcBlend = BF_One,
		EBlendFactor    RT0AlphaDestBlend = BF_Zero,
		EColorWriteMask RT1ColorWriteMask = CW_RGBA,
		EBlendOperation RT1ColorBlendOp = BO_Add,
		EBlendFactor    RT1ColorSrcBlend = BF_One,
		EBlendFactor    RT1ColorDestBlend = BF_Zero,
		EBlendOperation RT1AlphaBlendOp = BO_Add,
		EBlendFactor    RT1AlphaSrcBlend = BF_One,
		EBlendFactor    RT1AlphaDestBlend = BF_Zero,
		EColorWriteMask RT2ColorWriteMask = CW_RGBA,
		EBlendOperation RT2ColorBlendOp = BO_Add,
		EBlendFactor    RT2ColorSrcBlend = BF_One,
		EBlendFactor    RT2ColorDestBlend = BF_Zero,
		EBlendOperation RT2AlphaBlendOp = BO_Add,
		EBlendFactor    RT2AlphaSrcBlend = BF_One,
		EBlendFactor    RT2AlphaDestBlend = BF_Zero,
		EColorWriteMask RT3ColorWriteMask = CW_RGBA,
		EBlendOperation RT3ColorBlendOp = BO_Add,
		EBlendFactor    RT3ColorSrcBlend = BF_One,
		EBlendFactor    RT3ColorDestBlend = BF_Zero,
		EBlendOperation RT3AlphaBlendOp = BO_Add,
		EBlendFactor    RT3AlphaSrcBlend = BF_One,
		EBlendFactor    RT3AlphaDestBlend = BF_Zero,
		EColorWriteMask RT4ColorWriteMask = CW_RGBA,
		EBlendOperation RT4ColorBlendOp = BO_Add,
		EBlendFactor    RT4ColorSrcBlend = BF_One,
		EBlendFactor    RT4ColorDestBlend = BF_Zero,
		EBlendOperation RT4AlphaBlendOp = BO_Add,
		EBlendFactor    RT4AlphaSrcBlend = BF_One,
		EBlendFactor    RT4AlphaDestBlend = BF_Zero,
		EColorWriteMask RT5ColorWriteMask = CW_RGBA,
		EBlendOperation RT5ColorBlendOp = BO_Add,
		EBlendFactor    RT5ColorSrcBlend = BF_One,
		EBlendFactor    RT5ColorDestBlend = BF_Zero,
		EBlendOperation RT5AlphaBlendOp = BO_Add,
		EBlendFactor    RT5AlphaSrcBlend = BF_One,
		EBlendFactor    RT5AlphaDestBlend = BF_Zero,
		EColorWriteMask RT6ColorWriteMask = CW_RGBA,
		EBlendOperation RT6ColorBlendOp = BO_Add,
		EBlendFactor    RT6ColorSrcBlend = BF_One,
		EBlendFactor    RT6ColorDestBlend = BF_Zero,
		EBlendOperation RT6AlphaBlendOp = BO_Add,
		EBlendFactor    RT6AlphaSrcBlend = BF_One,
		EBlendFactor    RT6AlphaDestBlend = BF_Zero,
		EColorWriteMask RT7ColorWriteMask = CW_RGBA,
		EBlendOperation RT7ColorBlendOp = BO_Add,
		EBlendFactor    RT7ColorSrcBlend = BF_One,
		EBlendFactor    RT7ColorDestBlend = BF_Zero,
		EBlendOperation RT7AlphaBlendOp = BO_Add,
		EBlendFactor    RT7AlphaSrcBlend = BF_One,
		EBlendFactor    RT7AlphaDestBlend = BF_Zero/*,
		bool			bUseAlphaToCoverage = false*/
		>
	class TStaticBlendState : public TStaticStateRHI<
		TStaticBlendState<
			RT0ColorWriteMask,RT0ColorBlendOp,RT0ColorSrcBlend,RT0ColorDestBlend,RT0AlphaBlendOp,RT0AlphaSrcBlend,RT0AlphaDestBlend,
			RT1ColorWriteMask,RT1ColorBlendOp,RT1ColorSrcBlend,RT1ColorDestBlend,RT1AlphaBlendOp,RT1AlphaSrcBlend,RT1AlphaDestBlend,
			RT2ColorWriteMask,RT2ColorBlendOp,RT2ColorSrcBlend,RT2ColorDestBlend,RT2AlphaBlendOp,RT2AlphaSrcBlend,RT2AlphaDestBlend,
			RT3ColorWriteMask,RT3ColorBlendOp,RT3ColorSrcBlend,RT3ColorDestBlend,RT3AlphaBlendOp,RT3AlphaSrcBlend,RT3AlphaDestBlend,
			RT4ColorWriteMask,RT4ColorBlendOp,RT4ColorSrcBlend,RT4ColorDestBlend,RT4AlphaBlendOp,RT4AlphaSrcBlend,RT4AlphaDestBlend,
			RT5ColorWriteMask,RT5ColorBlendOp,RT5ColorSrcBlend,RT5ColorDestBlend,RT5AlphaBlendOp,RT5AlphaSrcBlend,RT5AlphaDestBlend,
			RT6ColorWriteMask,RT6ColorBlendOp,RT6ColorSrcBlend,RT6ColorDestBlend,RT6AlphaBlendOp,RT6AlphaSrcBlend,RT6AlphaDestBlend,
			RT7ColorWriteMask,RT7ColorBlendOp,RT7ColorSrcBlend,RT7ColorDestBlend,RT7AlphaBlendOp,RT7AlphaSrcBlend,RT7AlphaDestBlend/*,
			bUseAlphaToCoverage*/
			>,
		RHIBlendStateRef,
		RHIBlendState*
		>
	{
	public:
		static RHIBlendStateRef CreateRHI()
		{
			std::array<FBlendStateInitializer::FRenderTarget, 8> RenderTargetBlendStates;
			RenderTargetBlendStates[0] = FBlendStateInitializer::FRenderTarget(RT0ColorBlendOp,RT0ColorSrcBlend,RT0ColorDestBlend,RT0AlphaBlendOp,RT0AlphaSrcBlend,RT0AlphaDestBlend,RT0ColorWriteMask);
			RenderTargetBlendStates[1] = FBlendStateInitializer::FRenderTarget(RT1ColorBlendOp,RT1ColorSrcBlend,RT1ColorDestBlend,RT1AlphaBlendOp,RT1AlphaSrcBlend,RT1AlphaDestBlend,RT1ColorWriteMask);
			RenderTargetBlendStates[2] = FBlendStateInitializer::FRenderTarget(RT2ColorBlendOp,RT2ColorSrcBlend,RT2ColorDestBlend,RT2AlphaBlendOp,RT2AlphaSrcBlend,RT2AlphaDestBlend,RT2ColorWriteMask);
			RenderTargetBlendStates[3] = FBlendStateInitializer::FRenderTarget(RT3ColorBlendOp,RT3ColorSrcBlend,RT3ColorDestBlend,RT3AlphaBlendOp,RT3AlphaSrcBlend,RT3AlphaDestBlend,RT3ColorWriteMask);
			RenderTargetBlendStates[4] = FBlendStateInitializer::FRenderTarget(RT4ColorBlendOp,RT4ColorSrcBlend,RT4ColorDestBlend,RT4AlphaBlendOp,RT4AlphaSrcBlend,RT4AlphaDestBlend,RT4ColorWriteMask);
			RenderTargetBlendStates[5] = FBlendStateInitializer::FRenderTarget(RT5ColorBlendOp,RT5ColorSrcBlend,RT5ColorDestBlend,RT5AlphaBlendOp,RT5AlphaSrcBlend,RT5AlphaDestBlend,RT5ColorWriteMask);
			RenderTargetBlendStates[6] = FBlendStateInitializer::FRenderTarget(RT6ColorBlendOp,RT6ColorSrcBlend,RT6ColorDestBlend,RT6AlphaBlendOp,RT6AlphaSrcBlend,RT6AlphaDestBlend,RT6ColorWriteMask);
			RenderTargetBlendStates[7] = FBlendStateInitializer::FRenderTarget(RT7ColorBlendOp,RT7ColorSrcBlend,RT7ColorDestBlend,RT7AlphaBlendOp,RT7AlphaSrcBlend,RT7AlphaDestBlend,RT7ColorWriteMask);

			static RHIBlendStateRef RHI = GDynamicRHI->RHICreateBlendState(FBlendStateInitializer(RenderTargetBlendStates/*, bUseAlphaToCoverage*/));
			return RHI;
		}
	};
}