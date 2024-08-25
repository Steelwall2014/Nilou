#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <reflection/Class.h>
#include <shaderc/shaderc.h>

// #include <glm/glm.hpp>
#include "Common/Maths.h"
#include "Platform.h"
#include "RHIDefinitions.h"
#include "ShaderParameter.h"
#include "RHI.h"
#include "ShaderReflection.h"

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
		RHIShader(ERHIResourceType InResourceType) 
			: RHIResource(InResourceType)
		{ }
		shader_reflection::DescriptorSetLayouts Reflection;
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

	class RHIBuffer : public RHIResource
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
	};
	using RHIUniformBufferRef = std::shared_ptr<RHIUniformBuffer>;
	
	class RHITexture : public RHIResource 
	{
	public:
		RHITexture(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName, ETextureDimension InTextureType)
			: RHIResource(ERHIResourceType::RRT_Texture)
			, NumMips(InNumMips)
			, Format(InFormat)
			, TextureName(InTextureName)
			, TextureType(InTextureType)
			, SizeX(InSizeX)
			, SizeY(InSizeY)
			, SizeZ(InSizeZ)
		{ }
		virtual ~RHITexture() {}

		virtual uvec3 GetSizeXYZ() const { return uvec3(SizeX, SizeY, SizeZ); }
		uint32 GetSizeX() const { return SizeX; }
		uint32 GetSizeY() const { return SizeY; }
		uint32 GetSizeZ() const { return SizeZ; }
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
		ETextureDimension GetTextureType() const { return TextureType; };
		uint32 GetNumLayers() const { return TextureType == ETextureDimension::Texture2DArray || TextureType == ETextureDimension::TextureCube ? GetSizeXYZ().z : 1; }
	protected:
		uint32 NumMips;
		EPixelFormat Format;
		ETextureDimension TextureType;
		std::string TextureName;
		uint32 SizeX;
		uint32 SizeY;
		uint32 SizeZ;

	};
	using RHITextureRef = std::shared_ptr<RHITexture>;

	// Deprecated typenames
	using RHITexture2D = RHITexture;
	using RHITexture2DArray = RHITexture;
	using RHITexture3D = RHITexture;
	using RHITextureCube = RHITexture;
	using RHITexture2DRef = std::shared_ptr<RHITexture2D>;
	using RHITexture2DArrayRef = std::shared_ptr<RHITexture2DArray>;
	using RHITexture3DRef = std::shared_ptr<RHITexture3D>;
	using RHITextureCubeRef = std::shared_ptr<RHITextureCube>;

	class RHIShaderResourceView : public RHIResource
	{
	public:
		RHIShaderResourceView() : RHIResource(ERHIResourceType::RRT_ShaderResourceView) { }
	};

	class RHIUnorderedAccessView : public RHIResource
	{
	public:
		RHIUnorderedAccessView() : RHIResource(ERHIResourceType::RRT_UnorderedAccessView) { }
	};

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

	struct RenderTargetLayout
	{
		std::array<EPixelFormat, MAX_SIMULTANEOUS_RENDERTARGETS> RenderTargetFormats = { PF_Unknown };
		uint32 NumRenderTargetsEnabled = 0;
		EPixelFormat DepthStencilTargetFormat = PF_Unknown;

		bool operator==(const RenderTargetLayout &Other) const = default;
	};

	class FGraphicsPipelineStateInitializer
	{
	public: 
		FGraphicsPipelineStateInitializer()
			: VertexShader(nullptr)
			, PixelShader(nullptr)
			, DepthStencilState(nullptr)
			, RasterizerState(nullptr)
			, BlendState(nullptr)
			, VertexDeclaration(nullptr)
			, PrimitiveMode(EPrimitiveMode::PM_TriangleList)
		{ }

		RHIVertexShader *VertexShader;
		RHIPixelShader *PixelShader;

		EPrimitiveMode PrimitiveMode;

        RHIDepthStencilState* DepthStencilState;
        RHIRasterizerState* RasterizerState;
        RHIBlendState* BlendState;

		FRHIVertexDeclaration* VertexDeclaration;

		RenderTargetLayout RTLayout;

		bool operator==(const FGraphicsPipelineStateInitializer &Other) const = default;
	};

	using EDescriptorType = shader_reflection::EDescriptorType;

	struct RHIDescriptorSetLayoutBinding
	{
		uint32 BindingIndex;
		EDescriptorType DescriptorType;
		uint32 DescriptorCount = 1;		// For now, only support 1
	};

	class RHIDescriptorSetLayout : public RHIResource
	{
	public:
		friend class std::hash<RHIDescriptorSetLayout>;
		RHIDescriptorSetLayout() : RHIResource(RRT_DescriptorSetLayout) {}
		void GenerateHash();
		RHIDescriptorSetLayoutBinding* GetBindingIndexByName(const std::string &Name)
		{
			auto Found = NameToBindingIndex.find(Name);
			if (Found != NameToBindingIndex.end())
			{
				return &Bindings[Found->second];
			}
			return nullptr;
		}
		uint32 GetNumTypesUsed(EDescriptorType Type);

	protected:
		std::vector<RHIDescriptorSetLayoutBinding> Bindings;
		std::map<std::string, int> NameToBindingIndex;
		uint32 Hash;
	};
	using RHIDescriptorSetLayoutRef = std::shared_ptr<RHIDescriptorSetLayout>;

	constexpr uint32 VERTEX_SHADER_SET_INDEX = 0;
	constexpr uint32 PIXEL_SHADER_SET_INDEX = 1;
	constexpr uint32 VERTEX_FACTORY_SET_INDEX = 2;
	constexpr uint32 MATERIAL_SET_INDEX = 3;

	class RHIPipelineLayout : public RHIResource
	{
	public:
	 	RHIPipelineLayout() : RHIResource(RRT_PipelineLayout) {}
		struct UniformPosition
		{
			uint32 SetIndex;
			uint32 BindingIndex;
			uint32 Offset;
		};
		std::map<uint32, std::map<uint32, uint32>> UniformBuffersSize;
		std::map<std::string, UniformPosition> UniformPositions;
	};
	using RHIPipelineLayoutRef = std::shared_ptr<RHIPipelineLayout>;

	class FRHIPipelineState : public RHIResource
	{
	public: 
		FRHIPipelineState(const FGraphicsPipelineStateInitializer& InInitializer) 
			: RHIResource(RRT_PipelineState)
			, Initializer(InInitializer) {}

		RHIPipelineLayout* GetPipelineLayout() const
		{
			return PipelineLayout.get();
		}

		FGraphicsPipelineStateInitializer Initializer;

		// TODO: 可以改成unique_ptr？
		RHIPipelineLayoutRef PipelineLayout;

	};
	using FRHIPipelineStateRef = std::shared_ptr<FRHIPipelineState>;

	struct RHISamplerState : public RHIResource
	{
		RHISamplerState(const FSamplerStateInitializer& InInitializer) 
			: RHIResource(ERHIResourceType::RRT_SamplerState) 
			, Initializer(InInitializer) {}
		FSamplerStateInitializer Initializer;
	};
	using RHISamplerStateRef = std::shared_ptr<RHISamplerState>;

	class RHISampler
	{
	public:
		RHISampler();
		RHISampler(RHITexture* Texture);
		RHISampler(RHITexture* Texture, RHISamplerState* InSamplerState) 
			: SamplerState(InSamplerState)
			, Texture(Texture) 
		{}
		RHISamplerState* SamplerState;
		RHITexture* Texture;
	};
	using RHISamplerRef = std::shared_ptr<RHISampler>;

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

    class RHIDescriptorSet : public RHIResource
    {
    public:

		RHIDescriptorSet() : RHIResource(RRT_DescriptorSet) {}

        virtual void SetUniformBuffer(uint32 BindingIndex, RHIBuffer* Buffer) { }

        virtual void SetStorageBuffer(uint32 BindingIndex, RHIBuffer* Buffer) { }

        virtual void SetSampler(uint32 BindingIndex, RHISampler Sampler) { }

        virtual void SetStorageImage(uint32 BindingIndex, RHITexture* Image) { }

		class RHIDescriptorPool* GetPool() const;

    };
	using RHIDescriptorSetRef = std::shared_ptr<RHIDescriptorSet>;

	struct RHIDescriptorPoolSize
	{
		EDescriptorType Type;
		uint32 DescriptorCount;
	};
	class RHIDescriptorPool : public RHIResource
	{
	public:
		RHIDescriptorPool(RHIDescriptorSetLayout* InLayout)
			: RHIResource(RRT_DescriptorPool)
			, Layout(InLayout)
		{ }

		RHIDescriptorSetLayout* Layout;

		virtual RHIDescriptorSet* Allocate() { return nullptr;}
		virtual void Release(RHIDescriptorSet* DescriptorSet) { }

		bool CanAllocate() { return false; }
	};
	using RHIDescriptorPoolRef = std::shared_ptr<RHIDescriptorPool>;


}

namespace std {

template<>
struct hash<nilou::FGraphicsPipelineStateInitializer>
{
	size_t operator()(const nilou::FGraphicsPipelineStateInitializer &_Keyval) const noexcept;
};

template<>
struct hash<nilou::RHIDescriptorSetLayout>
{
	size_t operator()(const nilou::RHIDescriptorSetLayout &_Keyval) const noexcept
	{
		return _Keyval.Hash;
	}
};

} // namespace std