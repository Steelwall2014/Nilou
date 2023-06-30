#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <reflection/Class.h>

// #include <glm/glm.hpp>
#include "Common/Maths.h"
#include "Platform.h"
#include "RHIDefinitions.h"
#include "ShaderParameter.h"
#include "RHI.h"

namespace glslang {
class TShader;
}

namespace nilou {

	struct NSTRUCT RHITextureParams
	{
		GENERATED_STRUCT_BODY()
		
		NPROPERTY()
		ETextureFilters Mag_Filter;
		NPROPERTY()
		ETextureFilters Min_Filter;
		NPROPERTY()
		ETextureWrapModes Wrap_S; 
		NPROPERTY()
		ETextureWrapModes Wrap_T; 
		NPROPERTY()
		ETextureWrapModes Wrap_R;
		RHITextureParams(
			ETextureFilters InMagFilter=ETextureFilters::TF_Linear,
			ETextureFilters InMinFilter=ETextureFilters::TF_Linear_Mipmap_Linear,
			ETextureWrapModes InWrap_S=ETextureWrapModes::TW_Repeat,
			ETextureWrapModes InWrap_T=ETextureWrapModes::TW_Repeat,
			ETextureWrapModes InWrap_R=ETextureWrapModes::TW_Repeat)
			: Mag_Filter(InMagFilter)
			, Min_Filter(InMinFilter)
			, Wrap_S(InWrap_S)
			, Wrap_T(InWrap_T)
			, Wrap_R(InWrap_R)
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
		glslang::TShader* ShaderGlsl = nullptr;
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
		FRasterizerStateInitializer Initializer;
	};
	using RHIRasterizerStateRef = std::shared_ptr<RHIRasterizerState>;

	class RHIDepthStencilState : public RHIResource
	{
	public:
		RHIDepthStencilState() :RHIResource(ERHIResourceType::RRT_DepthStencilState) { }
		FDepthStencilStateInitializer Initializer;
	};
	using RHIDepthStencilStateRef = std::shared_ptr<RHIDepthStencilState>;

	class RHIBlendState : public RHIResource
	{
	public:
		RHIBlendState() :RHIResource(ERHIResourceType::RRT_BlendState) { }
		FBlendStateInitializer Initializer;
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

		virtual uvec3 GetSizeXYZ() const { return uvec3(0, 0, 0); }
		uint32 GetNumMips() const
		{
			return NumMips;
		}
		EPixelFormat GetFormat() const
		{
			return Format;
		}
		std::string GetName() const
		{
			return TextureName;
		}
		std::string SetName(const std::string &InTextureName)
		{
			TextureName = InTextureName;
		}
		ETextureType GetTextureType() const { return TextureType; };
		uint32 GetNumLayers() const { return TextureType == ETextureType::TT_Texture2DArray ? GetSizeXYZ().z : 1; }
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

		virtual uvec3 GetSizeXYZ() const override { return uvec3(SizeX, SizeY, 1); }
		uint32 GetSizeX() const { return SizeX; }
		uint32 GetSizeY() const { return SizeY; }

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

		virtual uvec3 GetSizeXYZ() const override { return uvec3(SizeX, SizeY, SizeZ); }
		uint32 GetSizeZ() const { return SizeZ; }

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

		virtual uvec3 GetSizeXYZ() const override { return uvec3(SizeX, SizeY, SizeZ); }
		uint32 GetSizeX() const { return SizeX; }
		uint32 GetSizeY() const { return SizeY; }
		uint32 GetSizeZ() const { return SizeZ; }

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

		virtual uvec3 GetSizeXYZ() const override { return uvec3(Size, Size, 1); }

	protected:
		uint32 Size;
	};
	using RHITextureCubeRef = std::shared_ptr<RHITextureCube>;

	class RHIFramebuffer : public RHIResource 
	{
	public:
		RHIFramebuffer() : RHIResource(ERHIResourceType::RRT_Framebuffer) { }
		virtual bool Check() = 0;
		virtual ~RHIFramebuffer() { }
		std::map<EFramebufferAttachment, RHITexture2DRef> Attachments;
	};
	using RHIFramebufferRef = std::shared_ptr<RHIFramebuffer>;

	class FRHIVertexDeclaration : public RHIResource
	{
	public:
		FRHIVertexDeclaration() : RHIResource(ERHIResourceType::RRT_VertexDeclaration) { }
	};
	using FRHIVertexDeclarationRef = std::shared_ptr<FRHIVertexDeclaration>;

	class FGraphicsPipelineStateInitializer
	{
	public: 
		FGraphicsPipelineStateInitializer()
			: VertexShader(nullptr)
			, PixelShader(nullptr)
			, ComputeShader(nullptr)
			, DepthStencilState(nullptr)
			, RasterizerState(nullptr)
			, BlendState(nullptr)
			, VertexDeclaration(nullptr)
			, PrimitiveMode(EPrimitiveMode::PM_TriangleList)
		{ }

		RHIVertexShader *VertexShader;
		RHIPixelShader *PixelShader;
		RHIComputeShader *ComputeShader;

		EPrimitiveMode PrimitiveMode;

        RHIDepthStencilState* DepthStencilState;
        RHIRasterizerState* RasterizerState;
        RHIBlendState* BlendState;

		FRHIVertexDeclaration* VertexDeclaration;

		std::array<EPixelFormat, MAX_SIMULTANEOUS_RENDERTARGETS> RenderTargetFormats = { EPixelFormat::PF_R8G8B8 };
		uint32 NumRenderTargetsEnabled;

		EPixelFormat DepthStencilTargetFormat;

		bool operator==(const FGraphicsPipelineStateInitializer &Other) const;

		void BuildRenderTargetFormats(RHIFramebuffer* Framebuffer);
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

	class FRHIPipelineLayout : public RHIResource
	{
	public:
	 	FRHIPipelineLayout() : RHIResource(RRT_PipelineLayout) {}
		FRHIDescriptorSet DescriptorSets[EPipelineStage::PipelineStageNum];
	};
	using FRHIPipelineLayoutRef = std::shared_ptr<FRHIPipelineLayout>;

	class FRHIGraphicsPipelineState : public RHIResource
	{
	public: 
		FRHIGraphicsPipelineState() : RHIResource(RRT_GraphicsPipelineState) {}

		FGraphicsPipelineStateInitializer Initializer;

		FRHIPipelineLayoutRef PipelineLayout;

		int GetBaseIndexByName(EPipelineStage PipelineStage, const std::string &Name);
	};
	using FRHIGraphicsPipelineStateRef = std::shared_ptr<FRHIGraphicsPipelineState>;

	struct RHISamplerState : public RHIResource
	{
		RHISamplerState() : RHIResource(ERHIResourceType::RRT_SamplerState) {}
		ETextureFilters Mag_Filter=TF_Linear;
		ETextureFilters Min_Filter=TF_Linear_Mipmap_Linear;
		ETextureWrapModes Wrap_S=TW_Repeat; 
		ETextureWrapModes Wrap_T=TW_Repeat; 
		ETextureWrapModes Wrap_R=TW_Repeat;
	};
	using RHISamplerStateRef = std::shared_ptr<RHISamplerState>;

	class FRHISampler
	{
	public:
		FRHISampler();
		FRHISampler(RHITexture* Texture);
		FRHISampler(RHITexture* Texture, RHISamplerState* InSamplerState) 
			: SamplerState(InSamplerState)
			, Texture(Texture) 
		{}
		FRHISampler(RHITextureRef Texture) : FRHISampler(Texture.get()) {}
		FRHISampler(RHITextureRef Texture, RHISamplerState* InSamplerState) : FRHISampler(Texture.get(), InSamplerState) {}
		RHISamplerState* SamplerState;
		RHITexture* Texture;
	};
	using FRHISamplerRef = std::shared_ptr<FRHISampler>;

	class FRHIRenderPassInfo
	{
	public:
		RHIFramebuffer *Framebuffer;
		ivec2 Viewport;
		bool bClearColorBuffer;
		vec4 ClearColor;
		bool bClearDepthBuffer;
		float ClearDepth;
		bool bClearStencilBuffer;
		uint32 ClearStencil;
		FRHIRenderPassInfo(
			RHIFramebuffer *InFramebuffer, 
			ivec2 InViewport,
			bool bInClearColorBuffer=false, 
			bool bInClearDepthBuffer=false, 
			bool bInClearStencilBuffer=false, 
			vec4 InClearColor=vec4(0.f, 0.f, 0.f, 1.0f), 
			float InClearDepth=1.0f,
			uint32 InClearStencil=0)
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

namespace std {

template<>
struct hash<nilou::FGraphicsPipelineStateInitializer>
{
	size_t operator()(const nilou::FGraphicsPipelineStateInitializer &_Keyval) const noexcept;
};

}