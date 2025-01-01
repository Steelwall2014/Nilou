#pragma once
#include "Platform.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "Templates/TypeHash.h"

namespace nilou {

class FRDGPass;

enum class ERDGResourceType
{
    Texture,
    TextureView,
    Buffer,
    UniformBuffer,
    DescriptorSet,
    MAX
};

enum class ERDGTextureUsage
{
    None,
    RenderTarget,
    ShaderResource,
};

enum class ERDGPooledBufferAlignment : uint8
{
	// The buffer size is not aligned.
	None,

	// The buffer size is aligned up to the next page size.
	Page,

	// The buffer size is aligned up to the next power of two.
	PowerOfTwo
};

using FRDGPassHandle = uint16;
constexpr FRDGPassHandle NullPassHandle = std::numeric_limits<FRDGPassHandle>::max();

using FRDGPassHandlesByPipeline = std::array<FRDGPassHandle, static_cast<size_t>(ERHIPipeline::Num)>;

using RDGTextureDesc = RHITextureDesc;

struct RDGBufferDesc
{
	/** Stride in bytes for index and structured buffers. */
	uint32 BytesPerElement = 1;

	/** Number of elements. */
	uint32 NumElements = 1;

	/** Bitfields describing the uses of that buffer. */
	EBufferUsageFlags Usage = EBufferUsageFlags::None;

	uint32 GetSize() const
	{
		return BytesPerElement * NumElements;
	}

	uint32 GetStride() const
	{
		return BytesPerElement;
	}

	RHIBufferDesc Translate() const
	{
		RHIBufferDesc Desc;
		Desc.Size = GetSize();
		Desc.Stride = GetStride();
		// Desc.Usage = Usage;
		return Desc;
	}

	bool operator==(const RDGBufferDesc& Other) const = default;
};

inline uint32 GetTypeHash(const RDGBufferDesc& Desc)
{
	uint32 Hash = GetTypeHash(Desc.BytesPerElement);
	Hash = HashCombine(Hash, GetTypeHash(Desc.NumElements));
	Hash = HashCombine(Hash, GetTypeHash(static_cast<int32>(Desc.Usage)));
	return Hash;
}

}

namespace std {

template<>
class hash<nilou::RDGBufferDesc>
{
public:
	size_t operator()(const nilou::RDGBufferDesc& Desc) const noexcept
	{
		uint32 Hash = nilou::GetTypeHash(Desc.BytesPerElement);
		Hash = nilou::HashCombine(Hash, nilou::GetTypeHash(Desc.NumElements));
		Hash = nilou::HashCombine(Hash, nilou::GetTypeHash(static_cast<int32>(Desc.Usage)));
		return Hash;
	}
};

}