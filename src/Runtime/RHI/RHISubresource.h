#pragma once
#include "Platform.h"
#include "RHIResources.h"

namespace nilou {

struct RHITextureSubresource
{
	RHITextureSubresource()
		: MipIndex(0)
		, PlaneSlice(0)
		, ArraySlice(0)
	{}

	RHITextureSubresource(uint32 InMipIndex, uint32 InArraySlice, uint32 InPlaneSlice)
		: MipIndex(InMipIndex)
		, PlaneSlice(InPlaneSlice)
		, ArraySlice(InArraySlice)
	{}

	inline bool operator == (const RHITextureSubresource& RHS) const
	{
		return MipIndex == RHS.MipIndex
			&& PlaneSlice == RHS.PlaneSlice
			&& ArraySlice == RHS.ArraySlice;
	}

	inline bool operator != (const RHITextureSubresource& RHS) const
	{
		return !(*this == RHS);
	}

	inline bool operator < (const RHITextureSubresource& RHS) const
	{
		return MipIndex < RHS.MipIndex
			&& PlaneSlice < RHS.PlaneSlice
			&& ArraySlice < RHS.ArraySlice;
	}

	inline bool operator <= (const RHITextureSubresource& RHS) const
	{
		return MipIndex <= RHS.MipIndex
			&& PlaneSlice <= RHS.PlaneSlice
			&& ArraySlice <= RHS.ArraySlice;
	}

	inline bool operator > (const RHITextureSubresource& RHS) const
	{
		return MipIndex > RHS.MipIndex
			&& PlaneSlice > RHS.PlaneSlice
			&& ArraySlice > RHS.ArraySlice;
	}

	inline bool operator >= (const RHITextureSubresource& RHS) const
	{
		return MipIndex >= RHS.MipIndex
			&& PlaneSlice >= RHS.PlaneSlice
			&& ArraySlice >= RHS.ArraySlice;
	}

	uint32 MipIndex   : 8;
	uint32 PlaneSlice : 8;
	uint32 ArraySlice : 16;
};

struct RHITextureSubresourceLayout
{
	RHITextureSubresourceLayout()
		: NumMips(0)
		, NumPlaneSlices(0)
		, NumArraySlices(0)
	{}

	RHITextureSubresourceLayout(uint32 InNumMips, uint32 InNumArraySlices, uint32 InNumPlaneSlices)
		: NumMips(InNumMips)
		, NumPlaneSlices(InNumPlaneSlices)
		, NumArraySlices(InNumArraySlices)
	{}

	RHITextureSubresourceLayout(const RHITextureDesc& Desc)
		: RHITextureSubresourceLayout(Desc.NumMips, Desc.ArraySize * (Desc.TextureType == ETextureDimension::TextureCube ? 6 : 1), IsStencilFormat(Desc.Format) ? 2 : 1)
	{}

	inline uint32 GetSubresourceCount() const
	{
		return NumMips * NumArraySlices * NumPlaneSlices;
	}

	inline uint32 GetSubresourceIndex(RHITextureSubresource Subresource) const
	{
		Ncheck(Subresource < GetMaxSubresource());
		return Subresource.MipIndex + (Subresource.ArraySlice * NumMips) + (Subresource.PlaneSlice * NumMips * NumArraySlices);
	}

	inline RHITextureSubresource GetSubresource(uint32 Index) const
	{
		RHITextureSubresource Subresource;
		Subresource.MipIndex = Index % NumMips;
		Subresource.ArraySlice = (Index / NumMips) % NumArraySlices;
		Subresource.PlaneSlice = Index / (NumMips * NumArraySlices);
		return Subresource;
	}

	inline RHITextureSubresource GetMaxSubresource() const
	{
		return RHITextureSubresource(NumMips, NumArraySlices, NumPlaneSlices);
	}

	inline bool operator == (RHITextureSubresourceLayout const& RHS) const
	{
		return NumMips == RHS.NumMips
			&& NumPlaneSlices == RHS.NumPlaneSlices
			&& NumArraySlices == RHS.NumArraySlices;
	}

	inline bool operator != (RHITextureSubresourceLayout const& RHS) const
	{
		return !(*this == RHS);
	}

	uint32 NumMips        : 8;
	uint32 NumPlaneSlices : 8;
	uint32 NumArraySlices : 16;
};

struct RHITextureSubresourceRange
{
	RHITextureSubresourceRange()
		: MipIndex(0)
		, PlaneSlice(0)
		, ArraySlice(0)
		, NumMips(0)
		, NumPlaneSlices(0)
		, NumArraySlices(0)
	{}

	explicit RHITextureSubresourceRange(RHITextureSubresourceLayout Layout)
		: MipIndex(0)
		, PlaneSlice(0)
		, ArraySlice(0)
		, NumMips(Layout.NumMips)
		, NumPlaneSlices(Layout.NumPlaneSlices)
		, NumArraySlices(Layout.NumArraySlices)
	{}

	bool operator == (RHITextureSubresourceRange const& RHS) const
	{
		return MipIndex == RHS.MipIndex
			&& PlaneSlice == RHS.PlaneSlice
			&& ArraySlice == RHS.ArraySlice
			&& NumMips == RHS.NumMips
			&& NumPlaneSlices == RHS.NumPlaneSlices
			&& NumArraySlices == RHS.NumArraySlices;
	}

	bool operator != (RHITextureSubresourceRange const& RHS) const
	{
		return !(*this == RHS);
	}

	inline uint32 GetSubresourceCount() const
	{
		return NumMips * NumArraySlices * NumPlaneSlices;
	}

	inline uint32 GetSubresourceIndex(RHITextureSubresource Subresource) const
	{
		Ncheck(Subresource >= GetMinSubresource());
		Ncheck(Subresource < GetMaxSubresource());
		Subresource.MipIndex -= MipIndex;
		Subresource.ArraySlice -= ArraySlice;
		Subresource.PlaneSlice -= PlaneSlice;
		return Subresource.MipIndex + (Subresource.ArraySlice * NumMips) + (Subresource.PlaneSlice * NumMips * NumArraySlices);
	}

	inline RHITextureSubresource GetSubresource(uint32 Index) const
	{
		RHITextureSubresource Subresource;
		Subresource.MipIndex = Index % NumMips + MipIndex;
		Subresource.ArraySlice = (Index / NumMips) % NumArraySlices + ArraySlice;
		Subresource.PlaneSlice = Index / (NumMips * NumArraySlices) + PlaneSlice;
		return Subresource;
	}

	RHITextureSubresource GetMinSubresource() const
	{
		return RHITextureSubresource(MipIndex, ArraySlice, PlaneSlice);
	}

	RHITextureSubresource GetMaxSubresource() const
	{
		return RHITextureSubresource(MipIndex + NumMips, ArraySlice + NumArraySlices, PlaneSlice + NumPlaneSlices);
	}

	template <typename T>
	struct iterator_base
	{
		T* Range;
		RHITextureSubresource Subresource;

		iterator_base(T* InRange, RHITextureSubresource InSubresource)
			: Range(InRange)
			, Subresource(InSubresource)
		{}

		iterator_base& operator++()
		{
			++Subresource.MipIndex;
			if (Subresource.MipIndex == Range->MipIndex + Range->NumMips)
			{
				Subresource.MipIndex = Range->MipIndex;
				++Subresource.ArraySlice;
				if (Subresource.ArraySlice == Range->ArraySlice + Range->NumArraySlices)
				{
					Subresource.ArraySlice = Range->ArraySlice;
					++Subresource.PlaneSlice;
				}
			}
			return *this;
		}

		bool operator==(const iterator_base& RHS) const
		{
			return Range == RHS.Range && Subresource == RHS.Subresource;
		}

		bool operator!=(const iterator_base& RHS) const
		{
			return !(*this == RHS);
		}

		RHITextureSubresource& operator*()
		{
			return Subresource;
		}
	};

	using iterator = iterator_base<RHITextureSubresourceRange>;
	using const_iterator = iterator_base<const RHITextureSubresourceRange>;

	auto begin() const
	{
		return const_iterator(this, GetMinSubresource());
	}

	auto begin()
	{
		return iterator(this, GetMinSubresource());
	}

	auto end() const
	{
		return const_iterator(this, GetMaxSubresource());
	}

	auto end()
	{
		return iterator(this, GetMaxSubresource());
	}

	bool IsWholeResource(const RHITextureSubresourceLayout& Layout) const
	{
		return MipIndex == 0
			&& PlaneSlice == 0
			&& ArraySlice == 0
			&& NumMips == Layout.NumMips
			&& NumPlaneSlices == Layout.NumPlaneSlices
			&& NumArraySlices == Layout.NumArraySlices;
	}

	bool IsValid(const RHITextureSubresourceLayout& Layout) const
	{
		return MipIndex + NumMips <= Layout.NumMips
			&& PlaneSlice + NumPlaneSlices <= Layout.NumPlaneSlices
			&& ArraySlice + NumArraySlices <= Layout.NumArraySlices;
	}

	uint32 MipIndex       : 8;
	uint32 PlaneSlice     : 8;
	uint32 ArraySlice     : 16;
	uint32 NumMips        : 8;
	uint32 NumPlaneSlices : 8;
	uint32 NumArraySlices : 16;
};

template <typename ElementType>
void VerifyLayout(const std::vector<ElementType>& SubresourceArray, const RHITextureSubresourceLayout& Layout)
{
	Ncheckf(Layout.GetSubresourceCount() > 0, "Subresource layout has no subresources.");
	Ncheckf(SubresourceArray.size() == Layout.GetSubresourceCount(), "Subresource array does not match the subresource layout.");
}

template <typename ElementType>
const ElementType& GetSubresource(const std::vector<ElementType>& SubresourceArray, const RHITextureSubresourceLayout& Layout, RHITextureSubresource Subresource)
{
	VerifyLayout(SubresourceArray, Layout);
	return SubresourceArray[Layout.GetSubresourceIndex(Subresource)];
}

template <typename ElementType>
ElementType& GetSubresource(std::vector<ElementType>& SubresourceArray, const RHITextureSubresourceLayout& Layout, RHITextureSubresource Subresource)
{
	VerifyLayout(SubresourceArray, Layout);
	return SubresourceArray[Layout.GetSubresourceIndex(Subresource)];
}

}