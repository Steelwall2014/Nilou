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


namespace nilou {

	struct RHITextureParams
	{
		ETextureFilters Mag_Filter;
		ETextureFilters Min_Filter;
		ETextureWrapModes Wrap_S; 
		ETextureWrapModes Wrap_T; 
		ETextureWrapModes Wrap_R;
		RHITextureParams()	// TODO 弄一个有默认参数的构造函数
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

		ERHIResourceType ResourceType;
	};

	class RHIShader : public RHIResource
	{
	public:
		RHIShader(ERHIResourceType InResourceType) : RHIResource(InResourceType) {}
		virtual bool Success() { return false; }
		virtual void ReleaseRHI() { }
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
			, ComputeShader(nullptr)
			, PrimitiveMode(EPrimitiveMode::PM_Triangles)
		{ }

		class FShaderInstance *VertexShader;
		class FShaderInstance *PixelShader;
		class FShaderInstance *ComputeShader;

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
			
		}

	private:
		std::tuple<
			FShaderInstance *const &, 
			FShaderInstance *const &, 
			FShaderInstance *const &, 
			const EPrimitiveMode&> tuplize() const
		{
			return std::tie(VertexShader, PixelShader, ComputeShader, PrimitiveMode);
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
		FRHISampler(RHITextureRef Texture, const RHITextureParams &Params=RHITextureParams::DefaultParams) 
			: RHIResource(ERHIResourceType::RRT_SamplerState)
			, Params(Params)
			, Texture(Texture.get()) 
		{}
		FRHISampler(RHITexture *Texture, const RHITextureParams &Params=RHITextureParams::DefaultParams) 
			: RHIResource(ERHIResourceType::RRT_SamplerState)
			, Params(Params)
			, Texture(Texture) 
		{}
		RHITextureParams Params;
		RHITexture* Texture;
	};
	using FRHISamplerRef = std::shared_ptr<FRHISampler>;

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
		ivec2 Viewport;
		bool bClearColorBuffer;
		glm::vec4 ClearColor;
		bool bClearDepthBuffer;
		float ClearDepth;
		bool bClearStencilBuffer;
		int ClearStencil;
		FRHIRenderPassInfo(
			RHIFramebuffer *InFramebuffer, 
			ivec2 InViewport,
			bool bInClearColorBuffer=false, 
			bool bInClearDepthBuffer=false, 
			bool bInClearStencilBuffer=false, 
			glm::vec4 InClearColor=glm::vec4(0.f, 0.f, 0.f, 1.0f), 
			float InClearDepth=1.0f,
			int InClearStencil=0)
			: Framebuffer(InFramebuffer)
			, Viewport(InViewport)
			, bClearColorBuffer(bInClearColorBuffer)
			, bClearDepthBuffer(bInClearDepthBuffer)
			, bClearStencilBuffer(bInClearStencilBuffer)
			, ClearColor(InClearColor)
			, ClearDepth(InClearDepth)
			, ClearStencil(InClearStencil)
		{
			
		}
	};

	class FRHIRenderQuery : public RHIResource
	{
	public:
		FRHIRenderQuery() : RHIResource(RRT_RenderQuery) {}
	};
	using FRHIRenderQueryRef = std::shared_ptr<FRHIRenderQuery>;

    struct DrawArraysIndirectCommand
    {
        int32 	Count;
        uint32 	instanceCount;
        uint32 	first;
        uint32 	baseInstance;
    };
    struct DrawElementsIndirectCommand
    {
        int32	Count;
        uint32 	instanceCount;
        uint32 	firstIndex;
        uint32 	baseVertex;
        uint32 	baseInstance;
    };
    struct DispatchIndirectCommand {
        uint32 	num_groups_x;
        uint32 	num_groups_y;
        uint32 	num_groups_z;
    };
}