#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include "Common/CoreUObject/Class.h"

// #include <glm/glm.hpp>
#include "Common/Math/Maths.h"
#include "Common/Crc.h"
#include "Platform.h"
#include "RHIDefinitions.h"
#include "ShaderParameter.h"
#include "RHI.h"
#include "ShaderReflection.h"
#include "Templates/TypeHash.h"
#include "Templates/RefCounting.h"

// TODO: Use intrusive shared pointers

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
	class RHIResource : public TRefCountedObject<ERefCountingMode::NotThreadSafe>
	{
	public:
		RHIResource(ERHIResourceType InResourceType) : ResourceType(InResourceType) {}
		virtual ~RHIResource() {};

		ERHIResourceType ResourceType;
	};

	struct RHIPushConstantRange
	{
		uint32 Size;
		struct Member
		{
			std::string Name;
			uint32 Offset;
			uint32 Size;
		};
		std::vector<Member> Members;
	};

	class RHIShader : public RHIResource
	{
	public:
		RHIShader(EShaderStage InShaderStage, ERHIResourceType InResourceType) 
			: RHIResource(InResourceType)
			, ShaderStage(InShaderStage)
		{ }
		std::string DebugName;
		std::unordered_map<uint32, TRefCountPtr<RHIDescriptorSetLayout>> DescriptorSetLayouts;
		std::optional<RHIPushConstantRange> PushConstantRange;
		EShaderStage ShaderStage;
		virtual bool Success() { return false; }
		virtual void ReleaseRHI() { }
	};
	using RHIShaderRef = TRefCountPtr<RHIShader>;
	
	class RHIVertexShader : public RHIShader 
	{
	public:
		RHIVertexShader() : RHIShader(EShaderStage::Vertex, ERHIResourceType::RRT_VertexShader) {}
	};
	using RHIVertexShaderRef = TRefCountPtr<RHIVertexShader>;
	
	class RHIPixelShader : public RHIShader 
	{
	public:
		RHIPixelShader() : RHIShader(EShaderStage::Pixel, ERHIResourceType::RRT_PixelShader) {}
	};
	using RHIPixelShaderRef = TRefCountPtr<RHIPixelShader>;
	
	class RHIComputeShader : public RHIShader 
	{
	public:
		RHIComputeShader() : RHIShader(EShaderStage::Compute, ERHIResourceType::RRT_ComputeShader) {}
	};
	using RHIComputeShaderRef = TRefCountPtr<RHIComputeShader>;

	class RHIRasterizerState : public RHIResource
	{
	public:
		RHIRasterizerState() :RHIResource(ERHIResourceType::RRT_RasterizerState) { }
		FRasterizerStateInitializer Initializer;
	};
	using RHIRasterizerStateRef = TRefCountPtr<RHIRasterizerState>;

	class RHIDepthStencilState : public RHIResource
	{
	public:
		RHIDepthStencilState() :RHIResource(ERHIResourceType::RRT_DepthStencilState) { }
		FDepthStencilStateInitializer Initializer;
	};
	using RHIDepthStencilStateRef = TRefCountPtr<RHIDepthStencilState>;

	class RHIBlendState : public RHIResource
	{
	public:
		RHIBlendState() :RHIResource(ERHIResourceType::RRT_BlendState) { }
		FBlendStateInitializer Initializer;
	};
	using RHIBlendStateRef = TRefCountPtr<RHIBlendState>;

	struct RHIBufferDesc
	{
		uint32 Size;
		uint32 Stride;
		EBufferUsageFlags Usage = EBufferUsageFlags::TransferSrc;

		RHIBufferDesc() = default;
		RHIBufferDesc(uint32 InSize, uint32 InStride, EBufferUsageFlags InUsage) : Size(InSize), Stride(InStride), Usage(InUsage) { }

		bool operator==(const RHIBufferDesc& Other) const = default;
	};
	using FRHIBufferCreateInfo = RHIBufferDesc;
	
	inline uint32 GetTypeHash(const RHIBufferDesc& Desc)
	{
		uint32 Hash = GetTypeHash(Desc.Size);
		Hash = HashCombine(Hash, GetTypeHash(Desc.Stride));
		Hash = HashCombine(Hash, GetTypeHash((uint32)Desc.Usage));
		return Hash;
	}

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
	using RHIBufferRef = TRefCountPtr<RHIBuffer>;

	class RHIUniformBuffer : public RHIResource
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
	using RHIUniformBufferRef = TRefCountPtr<RHIUniformBuffer>;
	
	struct RHITextureDesc
	{
		uint32 SizeX = 1;
		uint32 SizeY = 1;
		uint32 SizeZ = 1;
		uint32 ArraySize = 1;
		uint32 NumMips = 1;
		EPixelFormat Format = PF_Unknown;
		ETextureDimension TextureType = ETextureDimension::TextureDimensionsNum;
		ETextureUsageFlags Usage = ETextureUsageFlags::None;

		bool operator==(const RHITextureDesc& Other) const = default;
	};
	using FRHITextureCreateInfo = RHITextureDesc;
	
	inline uint32 GetTypeHash(const RHITextureDesc& Desc)
	{
		uint32 Hash = GetTypeHash(Desc.SizeX);
		Hash = HashCombine(Hash, GetTypeHash(Desc.SizeY));
		Hash = HashCombine(Hash, GetTypeHash(Desc.SizeZ));
		Hash = HashCombine(Hash, GetTypeHash(Desc.ArraySize));
		Hash = HashCombine(Hash, GetTypeHash(Desc.NumMips));
		Hash = HashCombine(Hash, GetTypeHash(Desc.Format));
		Hash = HashCombine(Hash, GetTypeHash(static_cast<int32>(Desc.TextureType)));
		Hash = HashCombine(Hash, GetTypeHash(static_cast<uint64>(Desc.Usage)));
		return Hash;
	}

	struct RHITextureViewDesc
	{
		EPixelFormat Format; 
		uint32 BaseMipLevel;
		uint32 LevelCount;
		uint32 BaseArrayLayer;
		uint32 LayerCount;
		ETextureDimension ViewType;

		bool operator==(const RHITextureViewDesc& Other) const = default;
	};
	using FRHITextureViewCreateInfo = RHITextureViewDesc;

	class FRHITextureViewCache
	{
	public:
		// Finds a UAV matching the descriptor in the cache or creates a new one and updates the cache.
		class RHITextureView* GetOrCreateView(class RHITexture* Texture, const FRHITextureViewCreateInfo& CreateInfo);

	private:
		std::vector<std::pair<FRHITextureViewCreateInfo, TRefCountPtr<RHITextureView>>> TextureViews;
	};

	class RHITexture : public RHIResource 
	{
	public:
		RHITexture(const std::string &InTextureName, RHITextureDesc InDesc)
			: RHIResource(ERHIResourceType::RRT_Texture)
			, Desc(InDesc)
			, TextureName(InTextureName)
		{ }
		virtual ~RHITexture() {}

		virtual uvec3 GetSizeXYZ() const { return uvec3(Desc.SizeX, Desc.SizeY, Desc.SizeZ); }
		uint32 GetSizeX() const { return Desc.SizeX; }
		uint32 GetSizeY() const { return Desc.SizeY; }
		uint32 GetSizeZ() const { return Desc.SizeZ; }
		uint32 GetNumMips() const
		{
			return Desc.NumMips;
		}
		EPixelFormat GetFormat() const
		{
			return Desc.Format;
		}
		std::string GetName() const
		{
			return TextureName;
		}
		const RHITextureDesc& GetDesc() const { return Desc; }
		void SetName(const std::string &InTextureName)
		{
			TextureName = InTextureName;
		}
		ETextureDimension GetTextureType() const { return Desc.TextureType; };
		uint32 GetNumLayers() const { return Desc.TextureType == ETextureDimension::Texture2DArray || Desc.TextureType == ETextureDimension::TextureCube ? GetSizeXYZ().z : 1; }

		RHITextureView* GetOrCreateView(const FRHITextureViewCreateInfo& CreateInfo) { return ViewCache.GetOrCreateView(this, CreateInfo); }

	protected:
		std::string TextureName;
		const RHITextureDesc Desc;

		FRHITextureViewCache ViewCache;

	};
	using RHITextureRef = TRefCountPtr<RHITexture>;

	// Deprecated typenames
	using RHITexture2D = RHITexture;
	using RHITexture2DArray = RHITexture;
	using RHITexture3D = RHITexture;
	using RHITextureCube = RHITexture;
	using RHITexture2DRef = TRefCountPtr<RHITexture2D>;
	using RHITexture2DArrayRef = TRefCountPtr<RHITexture2DArray>;
	using RHITexture3DRef = TRefCountPtr<RHITexture3D>;
	using RHITextureCubeRef = TRefCountPtr<RHITextureCube>;

	class RHITextureView : public RHIResource
	{
	public:
		RHITextureView(const RHITextureViewDesc& InDesc, RHITexture* InTexture) 
			: RHIResource(ERHIResourceType::RRT_TextureView) 
			, Desc(InDesc)
			, Texture(InTexture)
		{ }
		RHITextureViewDesc Desc;
		RHITexture* Texture;

		int32 GetSizeX() const { return Texture->GetSizeX() >> Desc.BaseMipLevel; }
		int32 GetSizeY() const { return Texture->GetSizeY() >> Desc.BaseMipLevel; }
		int32 GetSizeZ() const { return Texture->GetSizeZ() >> Desc.BaseMipLevel; }
	};
	using RHITextureViewRef = TRefCountPtr<RHITextureView>;

	class RHIFramebuffer : public RHIResource 
	{
	public:
		RHIFramebuffer() : RHIResource(ERHIResourceType::RRT_Framebuffer) { }
		virtual bool Check() = 0;
		virtual ~RHIFramebuffer() { }
		std::array<RHITextureView*, MaxSimultaneousRenderTargets> Attachments;
	};
	using RHIFramebufferRef = TRefCountPtr<RHIFramebuffer>;

	class FRHIVertexDeclaration : public RHIResource
	{
	public:
		FRHIVertexDeclaration() : RHIResource(ERHIResourceType::RRT_VertexDeclaration) { }
	};
	using FRHIVertexDeclarationRef = TRefCountPtr<FRHIVertexDeclaration>;

	struct RHIRenderTargetLayout
	{
		struct AttachmentDesc
		{
			EPixelFormat Format = PF_Unknown;
			ERenderTargetLoadAction LoadAction = ERenderTargetLoadAction::Load;
			ERenderTargetStoreAction StoreAction = ERenderTargetStoreAction::Store;
			bool operator==(const AttachmentDesc &Other) const = default;
		};
		std::array<AttachmentDesc, MaxSimultaneousRenderTargets> ColorAttachments;
		AttachmentDesc DepthStencilAttachment;

		bool operator==(const RHIRenderTargetLayout &Other) const = default;
	};

	class FGraphicsPipelineStateInitializer
	{
	public: 
		FGraphicsPipelineStateInitializer();

		RHIVertexShader *VertexShader;
		RHIPixelShader *PixelShader;

		EPrimitiveMode PrimitiveMode;

        RHIDepthStencilState* DepthStencilState;
        RHIRasterizerState* RasterizerState;
        RHIBlendState* BlendState;

		FRHIVertexDeclaration* VertexDeclaration;

		RHIRenderTargetLayout RTLayout;

		bool operator==(const FGraphicsPipelineStateInitializer &Other) const = default;

		uint32 ComputeNumValidRenderTargets() const
		{
			int32 LastValidIndex = -1;
			for (int32 i = MaxSimultaneousRenderTargets-1; i >= 0; i--)
			{
				if (RTLayout.ColorAttachments[i].Format != PF_Unknown)
				{
					LastValidIndex = i;
					break;
				}
			}
			return LastValidIndex + 1;
		}
	};

	using EDescriptorType = shader_reflection::EDescriptorType;

	enum class EDescriptorDecorationFlags
	{
		None = 0,
		// PushConstant = 1 << 0,
		NonWritable = 1 << 1,
		NonReadable = 1 << 2,
	};
	ENUM_CLASS_FLAGS(EDescriptorDecorationFlags);
	struct RHIDescriptorSetLayoutBinding
	{
		uint32 BindingIndex = 0;
		EDescriptorType DescriptorType = EDescriptorType::Max;
		uint32 DescriptorCount = 1;		// For now, only support 1

		std::string Name;
		uint32 BlockSize = 0;
		struct Member
		{
			std::string Name;
			uint32 Offset;
		};
		std::vector<Member> Members;
		EDescriptorDecorationFlags Flags = EDescriptorDecorationFlags::None;
	};

	class RHIDescriptorSetLayout : public RHIResource
	{
	public:
		RHIDescriptorSetLayout(const std::vector<RHIDescriptorSetLayoutBinding>& InBindings) 
			: RHIResource(RRT_DescriptorSetLayout)
			, Bindings(InBindings) 
		{
			std::sort(Bindings.begin(), Bindings.end(), [](const RHIDescriptorSetLayoutBinding& A, const RHIDescriptorSetLayoutBinding& B)
			{
				return A.BindingIndex < B.BindingIndex;
			});
		}
		uint32 GetNumTypeUsed(EDescriptorType Type) const
		{
			uint32 NumTypeUsed = 0;
			for (auto& Binding : Bindings)
			{
				NumTypeUsed += Binding.DescriptorType == Type;
			}
			return NumTypeUsed;
		}

		bool IsEquivalent(RHIDescriptorSetLayout* Other) const
		{
			if (Bindings.size() != Other->Bindings.size())
				return false;
			for (size_t i = 0; i < Bindings.size(); ++i)
			{
				if (Bindings[i].BindingIndex != Other->Bindings[i].BindingIndex)
					return false;
				if (Bindings[i].DescriptorType != Other->Bindings[i].DescriptorType)
					return false;
			}
			return true;
		}

		std::vector<RHIDescriptorSetLayoutBinding> Bindings;
	};
	using RHIDescriptorSetLayoutRef = TRefCountPtr<RHIDescriptorSetLayout>;

	class RHIPipelineLayout : public RHIResource
	{
	public:
	 	RHIPipelineLayout() : RHIResource(RRT_PipelineLayout) {}
    	std::unordered_map<uint32, RHIDescriptorSetLayout*> DescriptorSetLayouts;
		std::unordered_map<EShaderStage, RHIPushConstantRange> PushConstantRanges;
	};
	using RHIPipelineLayoutRef = TRefCountPtr<RHIPipelineLayout>;

	class RHIGraphicsPipelineState : public RHIResource
	{
	public: 
		RHIGraphicsPipelineState(const FGraphicsPipelineStateInitializer& InInitializer) 
			: RHIResource(RRT_PipelineState)
			, Initializer(InInitializer) {}

		RHIPipelineLayout* GetPipelineLayout() const
		{
			return PipelineLayout.GetReference();
		}

		FGraphicsPipelineStateInitializer Initializer;

		RHIPipelineLayoutRef PipelineLayout;

	};
	using RHIGraphicsPipelineStateRef = TRefCountPtr<RHIGraphicsPipelineState>;

	class RHIComputePipelineState : public RHIResource
	{
	public: 
		RHIComputePipelineState(RHIComputeShader* InComputeShader) 
			: RHIResource(RRT_PipelineState)
			, ComputeShader(InComputeShader) {}

		RHIPipelineLayout* GetPipelineLayout() const
		{
			return PipelineLayout.GetReference();
		}

		RHIComputeShader* ComputeShader;

		RHIPipelineLayoutRef PipelineLayout;

	};
	using RHIComputePipelineStateRef = TRefCountPtr<RHIComputePipelineState>;

	struct RHISamplerState : public RHIResource
	{
		RHISamplerState(const FSamplerStateInitializer& InInitializer) 
			: RHIResource(ERHIResourceType::RRT_SamplerState) 
			, Initializer(InInitializer) {}
		FSamplerStateInitializer Initializer;
	};
	using RHISamplerStateRef = TRefCountPtr<RHISamplerState>;

	class RHISampler
	{
	public:
		RHISampler(RHITexture* Texture=nullptr, RHISamplerState* InSamplerState=nullptr) 
			: SamplerState(InSamplerState)
			, Texture(Texture) 
		{}
		RHISamplerState* SamplerState;
		RHITexture* Texture;
	};

	class FRHIRenderPassInfo
	{
	public:
		RHIRenderTargetLayout RTLayout;

		std::array<RHITextureView*, MaxSimultaneousRenderTargets> ColorRenderTargets = { nullptr };
		std::array<vec4, MaxSimultaneousRenderTargets> ClearColors = { vec4(0.0f) };

		RHITextureView* DepthStencilRenderTarget = nullptr;
		float ClearDepth = 0.0f;
		uint32 ClearStencil = 0u;

		ivec2 Offset = ivec2(0);
		uvec2 Extent = uvec2(0);

		FRHIRenderPassInfo() { }
		FRHIRenderPassInfo(RHITextureView* RenderTarget, 
						   ERenderTargetLoadAction LoadAction=ERenderTargetLoadAction::Load, 
						   ERenderTargetStoreAction StoreAction=ERenderTargetStoreAction::Store)
		{
			ColorRenderTargets[0] = RenderTarget;
			RTLayout.ColorAttachments[0].LoadAction = LoadAction;
			RTLayout.ColorAttachments[0].StoreAction = StoreAction;
			Extent = uvec2(RenderTarget->GetSizeX(), RenderTarget->GetSizeY());
		}


	};

	class FRHIRenderQuery : public RHIResource
	{
	public:
		FRHIRenderQuery() : RHIResource(RRT_RenderQuery) {}
	};
	using FRHIRenderQueryRef = TRefCountPtr<FRHIRenderQuery>;

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

		RHIDescriptorSet(class RHIDescriptorPool* InPool) : RHIResource(RRT_DescriptorSet), Pool(InPool) {}

        virtual void SetUniformBuffer(uint32 BindingIndex, RHIBuffer* Buffer) { }

        virtual void SetStorageBuffer(uint32 BindingIndex, RHIBuffer* Buffer) { }

        virtual void SetSampler(uint32 BindingIndex, RHITextureView* Texture, RHISamplerState* SamplerState) { }

        virtual void SetStorageImage(uint32 BindingIndex, RHITextureView* InTexture) { }

		RHIDescriptorPool* GetPool() const { return Pool; }

		RHIDescriptorSetLayout* GetLayout() const;

		RHIDescriptorPool* Pool;
    };
	using RHIDescriptorSetRef = TRefCountPtr<RHIDescriptorSet>;

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
		virtual void Free(RHIDescriptorSet* DescriptorSet) { }
		virtual bool CanAllocate() const { return false; }
	};
	using RHIDescriptorPoolRef = TRefCountPtr<RHIDescriptorPool>;


}

namespace std {

template<>
struct hash<nilou::FGraphicsPipelineStateInitializer>
{
	size_t operator()(const nilou::FGraphicsPipelineStateInitializer &_Keyval) const noexcept;
};

template<>
struct hash<nilou::RHITextureDesc>
{
	size_t operator()(const nilou::RHITextureDesc &_Keyval) const noexcept
	{
		return GetTypeHash(_Keyval);
	}
};

template<>
struct hash<nilou::RHIBufferDesc>
{
	size_t operator()(const nilou::RHIBufferDesc &_Keyval) const noexcept
	{
		return GetTypeHash(_Keyval);
	}
};

} // namespace std