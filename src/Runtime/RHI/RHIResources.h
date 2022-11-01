#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <map>
#include <vector>

// #include <glm/glm.hpp>
#include "Common/Maths.h"
#include "Platform.h"
#include "RHIDefinitions.h"
#include "ShaderParameter.h"

namespace std {
	// class FRHIGraphicsPipelineInitializer;

	// template<class T>
	// inline  void  hash_combine(std::size_t & seed, const T & val){
	// 	seed ^= hash<T>()(val)+0x9e3779b9 + (seed << 6) + (seed >> 2);
	// }
	// template<class T>
	// inline  void  hash_val(std::size_t & seed, const T & val){
	// 	hash_combine(seed, val);
	// }
	
	// template<class T,class ...Types>
	// inline  void  hash_val(std::size_t & seed, const T & val,const Types & ...args){
	// 	hash_combine(seed, val);
	// 	hash_val(seed, args...);
	// }
	
	
	// template<class ...Types>
	// inline  size_t  hash_val(const Types & ...args){
	// 	size_t  seed = 0;
	// 	hash_val(seed, args...);
	// 	return seed;
	// }

	// template <>
	// struct hash<FRHIGraphicsPipelineInitializer> {
	// 	size_t operator()(const FRHIGraphicsPipelineInitializer &_Keyval) const noexcept {
	// 		return hash_val(_Keyval);
	// 	}
	// };
	// template<>
	// inline size_t hash<FRHIGraphicsPipelineInitializer>(const FRHIGraphicsPipelineInitializer &StateData)
	// {

	// }
}



namespace nilou {


	// struct RHIShaderResourceView
	// {
	// 	uint32 AttributeIndex;
	// 	uint32 Stride;
	// 	uint32 Offset;
	// 	EVertexElementTypeFlags Type;

	// 	RHIShaderResourceView()
	// 	: AttributeIndex(0)
	// 	, Stride(0)
	// 	, Offset(0)
	// 	, Type(EVertexElementTypeFlags::VET_None)
	// 	{ }
	// 	RHIShaderResourceView(
	// 		uint32 InAttributeIndex,
	// 		uint32 InStride,
	// 		uint32 InOffset,
	// 		EVertexElementTypeFlags InType
	// 	)
	// 	: AttributeIndex(InAttributeIndex)
	// 	, Stride(InStride)
	// 	, Offset(InOffset)
	// 	, Type(InType)
	// 	{ }

	// };
	// inline bool operator<(const RHIShaderResourceView &a, const RHIShaderResourceView &b)
	// {
	// 	return a.AttributeIndex < b.AttributeIndex;
	// }
	struct RHITextureParams
	{
		ETextureFilters Mag_Filter;
		ETextureFilters Min_Filter;
		ETextureWrapModes Wrap_S; 
		ETextureWrapModes Wrap_T; 
		ETextureWrapModes Wrap_R;
		RHITextureParams()
			: Mag_Filter(ETextureFilters::TF_Linear)
			, Min_Filter(ETextureFilters::TF_Linear)
			, Wrap_S(ETextureWrapModes::TW_Repeat)
			, Wrap_T(ETextureWrapModes::TW_Repeat)
			, Wrap_R(ETextureWrapModes::TW_Repeat)
		{}
		static RHITextureParams DefaultParams;
	};
	class RHIResource
	{
	public:
		RHIResource(ERHIResourceType InResourceType) : ResourceType(InResourceType) {}
		virtual ~RHIResource() {};
		friend class FDynamicRHI;

		ERHIResourceType ResourceType;
	};

	class RHIShader : public RHIResource
	{
	public:
		RHIShader(ERHIResourceType InResourceType) : RHIResource(InResourceType) {}
		virtual bool Success() { return false; }
	};
	using RHIShaderRef = std::shared_ptr<RHIShader>;
	
	class RHIVertexShader : public RHIShader 
	{
	public:
		RHIVertexShader() : RHIShader(ERHIResourceType::RRT_VertexShader) {}
	};
	using RHIVertexShaderRef = std::shared_ptr<RHIVertexShader>;
	
	class RHIPixelShader : public RHIShader 
	{
	public:
		RHIPixelShader() : RHIShader(ERHIResourceType::RRT_PixelShader) {}
	};
	using RHIPixelShaderRef = std::shared_ptr<RHIPixelShader>;
	
	class RHIComputeShader : public RHIShader 
	{
	public:
		RHIComputeShader() : RHIShader(ERHIResourceType::RRT_ComputeShader) {}
	};
	using RHIComputeShaderRef = std::shared_ptr<RHIComputeShader>;

	// class RHILinkedProgram : public RHIResource 
	// {
	// public:
	// 	RHIVertexShaderRef	VertexShader;
	// 	RHIPixelShaderRef	PixelShader;
	// 	RHIComputeShaderRef  ComputeShader;
	// 	RHILinkedProgram(RHIComputeShaderRef InComputeShader) 
	// 	: RHIResource(ERHIResourceType::RRT_LinkedProgram)
	// 	, VertexShader(nullptr)
	// 	, PixelShader(nullptr)
	// 	, ComputeShader(InComputeShader) 
	// 	{ }
	// 	RHILinkedProgram(RHIVertexShaderRef InVertexShader, RHIPixelShaderRef InPixelShader)
	// 	: RHIResource(ERHIResourceType::RRT_LinkedProgram)
	// 	, VertexShader(InVertexShader)
	// 	, PixelShader(InPixelShader)
	// 	, ComputeShader(nullptr) 
	// 	{ }
	// 	virtual bool Success() { return false; }
	// };
	// using RHILinkedProgramRef = std::shared_ptr<RHILinkedProgram>;


	// class RHIRasterizerState : public RHIResource
	// {
	// public:
	// 	bool EnableCull;
	// 	ERasterizerPolyMode_Face PolyMode_Face;
	// 	ERasterizerFillMode PolyMode_Mode;
	// 	ERasterizerCullMode CullMode;
	// 	ERasterizerFrontFace FrontFace;
	// 	RHIRasterizerState(
	// 		bool EnableCull = true, 
	// 		ERasterizerPolyMode_Face PolygonMode_Face = ERasterizerPolyMode_Face::PMF_Front_and_Back, ERasterizerFillMode PolygonMode_Mode = PMM_FILL,
	// 		ERasterizerCullMode Cull_Mode = CM_Back, ERasterizerFrontFace Front_Face = ERasterizerFrontFace::FF_CCW)
	// 		: RHIResource(ERHIResourceType::RRT_RasterizerState)
	// 		, EnableCull(EnableCull)
	// 		, PolyMode_Face(PolygonMode_Face)
	// 		, PolyMode_Mode(PolygonMode_Mode)
	// 		, CullMode(Cull_Mode)
	// 		, FrontFace(Front_Face)
	// 	{
	// 	}

	// 	bool operator==(const RHIRasterizerState &Other)
	// 	{
	// 		return EnableCull == Other.EnableCull &&
	// 				PolyMode_Face == Other.PolyMode_Face &&
	// 				PolyMode_Mode == Other.PolyMode_Mode &&
	// 				CullMode == Other.CullMode &&
	// 				FrontFace == Other.FrontFace;
	// 	}
	// };

	class RHIRasterizerState : public RHIResource
	{
	public:
		RHIRasterizerState() :RHIResource(ERHIResourceType::RRT_RasterizerState) { }
		virtual bool Equals(RHIRasterizerState *) { return false; }
	};
	using RHIRasterizerStateRef = std::shared_ptr<RHIRasterizerState>;

	class RHIDepthStencilState : public RHIResource
	{
	public:
		RHIDepthStencilState() :RHIResource(ERHIResourceType::RRT_DepthStencilState) { }
		virtual bool Equals(RHIDepthStencilState *) { return false; }
	};
	using RHIDepthStencilStateRef = std::shared_ptr<RHIDepthStencilState>;

	class RHIBlendState : public RHIResource
	{
	public:
		RHIBlendState() :RHIResource(ERHIResourceType::RRT_BlendState) { }
		virtual bool Equals(RHIBlendState *) { return false; }
	};
	using RHIBlendStateRef = std::shared_ptr<RHIBlendState>;


	// template<
	// 	bool bEnableDepthWrite,
	// 	ECompareFunction DepthTest,
	// 	bool bEnableFrontFaceStencil,
	// 	ECompareFunction FrontFaceStencilTest,
	// 	EStencilOp FrontFaceStencilFailStencilOp,
	// 	EStencilOp FrontFaceDepthFailStencilOp,
	// 	EStencilOp FrontFacePassStencilOp,
	// 	bool bEnableBackFaceStencil,
	// 	ECompareFunction BackFaceStencilTest,
	// 	EStencilOp BackFaceStencilFailStencilOp,
	// 	EStencilOp BackFaceDepthFailStencilOp,
	// 	EStencilOp BackFacePassStencilOp,
	// 	uint8 StencilReadMask,
	// 	uint8 StencilWriteMask
	// 	>
	// RHIDepthStencilStateRef TStaticDepthStencilState<
	// 	bEnableDepthWrite,
	// 	DepthTest,
	// 	bEnableFrontFaceStencil,
	// 	FrontFaceStencilTest,
	// 	FrontFaceStencilFailStencilOp,
	// 	FrontFaceDepthFailStencilOp,
	// 	FrontFacePassStencilOp,
	// 	bEnableBackFaceStencil,
	// 	BackFaceStencilTest,
	// 	BackFaceStencilFailStencilOp,
	// 	BackFaceDepthFailStencilOp,
	// 	BackFacePassStencilOp,
	// 	StencilReadMask,
	// 	StencilWriteMask>::DepthStencilStateRHI = std::make_shared<RHIDepthStencilState>();

	class RHIBuffer : RHIResource
	{
	public:
		/**
		* @param InSize: The number of bytes in the index buffer
		* @param InUsage: e.g. VertexBuffer | Static
		*/
		RHIBuffer(uint32 InStride, uint32 InSize, EBufferUsageFlags InUsage)
		: RHIResource(ERHIResourceType::RRT_Buffer)
		, Stride(InStride)
		, Size(InSize)
		, Usage(InUsage)
		{ }
		virtual ~RHIBuffer() {}
		uint32 GetStride() const { return Stride; }
		uint32 GetSize() const { return Size; }
		EBufferUsageFlags GetUsage() const { return Usage; }
		uint32 GetCount() const { return GetSize() / GetStride(); }
	protected:
		uint32 Stride;
		uint32 Size;
		EBufferUsageFlags Usage;
	};
	using RHIBufferRef = std::shared_ptr<RHIBuffer>;

	class RHIUniformBuffer : RHIResource
	{
	public:
		/**
		* @param InSize: The number of bytes in the uniform buffer
		* @param InUsage: e.g. UniformBuffer_SingleFrame
		*/
		RHIUniformBuffer(uint32 InSize, EUniformBufferUsage InUsage)
		: RHIResource(ERHIResourceType::RRT_UniformBuffer)
		, Size(InSize)
		, Usage(InUsage)
		{ }
		virtual ~RHIUniformBuffer() {}
		uint32 GetSize() const { return Size; }
		EUniformBufferUsage GetUsage() const { return Usage; }
	protected:
		uint32 Size;
		EUniformBufferUsage Usage;
		class FUniformBufferStructDeclaration *Declaration;
	};
	using RHIUniformBufferRef = std::shared_ptr<RHIUniformBuffer>;

	// class RHIVertexArrayObject : public RHIResource 
	// {
	// public:
	// 	EPrimitiveMode	Mode;
	// 	RHIBufferRef	IndexBuffer;
	// 	std::map<RHIShaderResourceView, RHIBufferRef>	VertexAttribs;
	// 	RHIVertexArrayObject(EPrimitiveMode InMode) 
	// 		: RHIResource(ERHIResourceType::RRT_VertexArrayObject)
	// 		, Mode(InMode) {}
	// 	virtual void AddVertexAttribBuffer(const RHIShaderResourceView &SRV, RHIBufferRef VertexAttri) {};
	// 	virtual void SetIndexBuffer(RHIBufferRef IndexBuffer) {};
	// 	virtual int32 GetPointCount() { return 0; };
	// 	virtual ~RHIVertexArrayObject() { }

	// };
	// using RHIVertexArrayObjectRef = std::shared_ptr<RHIVertexArrayObject>;
	
	class RHITexture : public RHIResource 
	{
	public:
		RHITexture(uint32 InNumMips, EPixelFormat InFormat, std::string InTextureName)
			: RHIResource(ERHIResourceType::RRT_Texture)
			, NumMips(InNumMips)
			, Format(InFormat)
			, TextureName(InTextureName)
		{ }
		virtual ~RHITexture() {}

		virtual glm::uvec3 GetSizeXYZ() { return glm::uvec3(0, 0, 0); }
		inline uint32 GetNumMips()
		{
			return NumMips;
		}
		inline EPixelFormat GetFormat()
		{
			return Format;
		}
		inline std::string GetName()
		{
			return TextureName;
		}
		inline std::string SetName(const std::string &InTextureName)
		{
			TextureName = InTextureName;
		}
		inline ETextureType GetTextureType() { return TextureType; };
	protected:
		uint32 NumMips;
		EPixelFormat Format;
		ETextureType TextureType;
		std::string TextureName;

	};
	using RHITextureRef = std::shared_ptr<RHITexture>;
	
	class RHITexture2D : public RHITexture 
	{
	public:
		RHITexture2D(uint32 InSizeX, uint32 InSizeY, uint32 InNumMips, EPixelFormat InFormat, std::string InTextureName)
		: RHITexture(InNumMips, InFormat, InTextureName)
		, SizeX(InSizeX)
		, SizeY(InSizeY)
		{ TextureType = ETextureType::TT_Texture2D; }

		inline virtual glm::uvec3 GetSizeXYZ() { return glm::uvec3(SizeX, SizeY, 1); }
		inline uint32 GetSizeX() { return SizeX; }
		inline uint32 GetSizeY() { return SizeY; }

	protected:
		uint32 SizeX;
		uint32 SizeY;
	};
	using RHITexture2DRef = std::shared_ptr<RHITexture2D>;
	
	class RHITexture2DArray : public RHITexture2D 
	{
	public:
		RHITexture2DArray(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, std::string InTextureName)
		: RHITexture2D(InSizeX, InSizeY, InNumMips, InFormat, InTextureName)
		, SizeZ(InSizeZ)
		{ TextureType = ETextureType::TT_Texture2DArray; }

		inline virtual glm::uvec3 GetSizeXYZ() { return glm::uvec3(SizeX, SizeY, SizeZ); }
		inline uint32 GetSizeZ() { return SizeZ; }

	private:
		uint32 SizeZ;
	};
	using RHITexture2DArrayRef = std::shared_ptr<RHITexture2DArray>;

	class RHITexture3D : public RHITexture
	{
	public:
		RHITexture3D(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, std::string InTextureName)
		: RHITexture(InNumMips, InFormat, InTextureName)
		, SizeX(InSizeX)
		, SizeY(InSizeY)
		, SizeZ(InSizeZ)
		{ TextureType = ETextureType::TT_Texture3D; }

		inline virtual glm::uvec3 GetSizeXYZ() { return glm::uvec3(SizeX, SizeY, SizeZ); }
		inline uint32 GetSizeX() { return SizeX; }
		inline uint32 GetSizeY() { return SizeY; }
		inline uint32 GetSizeZ() { return SizeZ; }

	protected:
		uint32 SizeX;
		uint32 SizeY;
		uint32 SizeZ;

	};
	using RHITexture3DRef = std::shared_ptr<RHITexture3D>;
	
	class RHITextureCube : public RHITexture 
	{
	public:
		RHITextureCube(uint32 InSize, uint32 InNumMips, EPixelFormat InFormat, std::string InTextureName)
		: RHITexture(InNumMips, InFormat, InTextureName)
		, Size(InSize)
		{ TextureType = ETextureType::TT_TextureCube; }

		inline virtual glm::uvec3 GetSizeXYZ() { return glm::uvec3(Size, Size, 1); }

	protected:
		uint32 Size;
	};
	using RHITextureCubeRef = std::shared_ptr<RHITextureCube>;

	// class FRHISamplerState : public RHIResource 
	// {
	// public:
	// 	FRHISamplerState() : RHIResource(RRT_SamplerState) {}
	// };

	class RHIFramebuffer : public RHIResource 
	{
	public:
		RHIFramebuffer() : RHIResource(ERHIResourceType::RRT_Framebuffer) { }
		virtual void AddAttachment(EFramebufferAttachment attachment, RHITexture2DRef texture) = 0;
		virtual bool Check() = 0;
		virtual ~RHIFramebuffer() { }
		virtual bool HasColorAttachment() = 0;
		virtual bool HasDepthAttachment() = 0;
		std::map<EFramebufferAttachment, RHITextureRef> Attachments;
	};
	using RHIFramebufferRef = std::shared_ptr<RHIFramebuffer>;

    class FRHIVertexDeclaration : public RHIResource
    {
    public:
        FRHIVertexDeclaration() : RHIResource(ERHIResourceType::RRT_VertexDeclaration) {}
    };
	using FRHIVertexDeclarationRef = std::shared_ptr<FRHIVertexDeclaration>;

	class FRHIGraphicsPipelineInitializer
	{
	public: 
		FRHIGraphicsPipelineInitializer()
			: VertexShader(nullptr)
			, PixelShader(nullptr)
			, PrimitiveMode(EPrimitiveMode::PM_Triangles)
		{ }

		class FShaderInstance *VertexShader;
		class FShaderInstance *PixelShader;

		EPrimitiveMode PrimitiveMode;

		bool operator==(const FRHIGraphicsPipelineInitializer &Other) const
		{
			return 	VertexShader == Other.VertexShader &&
					PixelShader == Other.PixelShader &&
					PrimitiveMode == Other.PrimitiveMode;
		}
		

		bool operator<(const FRHIGraphicsPipelineInitializer &Other) const
		{
			return this->tuplize() < Other.tuplize();
			// if (VertexShader < Other.VertexShader)
			// 	return true;
			// else if (VertexShader == Other.VertexShader)
			// 	if (PixelShader < Other.PixelShader)
			// 		return true;
			// 	else if (PixelShader == Other.PixelShader)
			// 		return PrimitiveMode < Other.PrimitiveMode;
			// 	else 
			// 		return false;
			// else 
			// 	return false;
			
		}

	private:
		std::tuple<
			FShaderInstance *const &, 
			FShaderInstance *const &, 
			const EPrimitiveMode&> tuplize() const
		{
			return std::tie(VertexShader, PixelShader, PrimitiveMode);
		}
	};

	class FRHIDescriptorSetLayoutBinding
	{
	public:
		std::string Name;
		int32 BindingPoint;
		EShaderParameterType ParameterType;
		// int32 DiscriptorCount;
	};

	class FRHIDescriptorSet
	{
	public:
		std::map<std::string, FRHIDescriptorSetLayoutBinding> Bindings;
	};

	class FRHIPipelineLayout
	{
	public:
		FRHIDescriptorSet DescriptorSets[EPipelineStage::PipelineStageNum];
	};

	class FRHIGraphicsPipelineState : public RHIResource
	{
	public: 
		FRHIGraphicsPipelineState() : RHIResource(RRT_GraphicsPipelineState) {}

		FRHIGraphicsPipelineInitializer Initializer;

		FRHIPipelineLayout PipelineLayout;

		int GetBaseIndexByName(EPipelineStage PipelineStage, const std::string &Name);
	};
	using FRHIGraphicsPipelineStateRef = std::shared_ptr<FRHIGraphicsPipelineState>;

	class FRHISampler : public RHIResource
	{
	public:
		FRHISampler() : RHIResource(ERHIResourceType::RRT_SamplerState) {}
		FRHISampler(const RHITextureParams &Params, RHITextureRef Texture) 
			: RHIResource(ERHIResourceType::RRT_SamplerState)
			, Params(Params)
			, Texture(Texture) 
		{}
		RHITextureParams Params;
		RHITextureRef Texture;
	};
	using FRHISamplerRef = std::shared_ptr<FRHISampler>;

	// class FRHISampler2D : public FRHISampler
	// {
	// public:
	// 	RHITexture2DRef Texture;
	// };

	// class FRHISampler3D : public FRHISampler
	// {
	// public:
	// 	RHITexture3DRef Texture;
	// };

	class FRHIVertexInput
    {
    public:
        RHIBuffer *VertexBuffer;
		uint8 Location;
        uint8 Offset;
        uint8 Stride;
        EVertexElementType Type;

        FRHIVertexInput() 
            : VertexBuffer(nullptr)
            , Offset(0)
            , Stride(0)
            , Type(EVertexElementType::VET_None)
        { }
    };

	class FRHIRenderPassInfo
	{
	public:
		RHIFramebuffer *Framebuffer;
		bool bClearColorBuffer;
		glm::vec4 ClearColor;
		bool bClearDepthBuffer;
		float ClearDepth;
		bool bClearStencilBuffer;
		int ClearStencil;
		FRHIRenderPassInfo(
			RHIFramebuffer *InFramebuffer, 
			bool bInClearColorBuffer=false, 
			bool bInClearDepthBuffer=false, 
			bool bInClearStencilBuffer=false, 
			glm::vec4 InClearColor=glm::vec4(0.f, 0.f, 0.f, 1.0f), 
			float InClearDepth=1.0f,
			int InClearStencil=0)
			: Framebuffer(InFramebuffer)
			, bClearColorBuffer(bInClearColorBuffer)
			, bClearDepthBuffer(bInClearDepthBuffer)
			, bClearStencilBuffer(bInClearStencilBuffer)
			, ClearColor(InClearColor)
			, ClearDepth(InClearDepth)
			, ClearStencil(InClearStencil)
		{
			
		}
	};
}

// namespace std {
// 	template<>
// 	struct hash<nilou::FRHIGraphicsPipelineInitializer>
// 	{
// 		size_t operator()(const nilou::FRHIGraphicsPipelineInitializer &Object)
// 		{

// 		}
// 	};
// }