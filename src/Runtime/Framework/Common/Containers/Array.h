#pragma once
#include <utility>
#include <iterator>
#include "Platform.h"
#include "Common/CoreUObject/Class.h"

namespace nilou {
    template <typename InElementType, uint32 NumElements, uint32 Alignment = alignof(InElementType)>
    class alignas(Alignment) TAlignedStaticArray
    {
    public:
        TAlignedStaticArray() { }

        InElementType &operator[](size_t index)
        {
            return Elements[index].Element;
        }

        const InElementType &operator[](size_t index) const
        {
            return Elements[index].Element;
        }

        // friend bool operator==(const TAlignedStaticArray& A,const TAlignedStaticArray& B)
        // {
        //     for(uint32 ElementIndex = 0;ElementIndex < NumElements;++ElementIndex)
        //     {
        //         if(!(A[ElementIndex] == B[ElementIndex]))
        //         {
        //             return false;
        //         }
        //     }
        //     return true;
        // }

        // friend bool operator!=(const TAlignedStaticArray& A,const TAlignedStaticArray& B)
        // {
        //     for(uint32 ElementIndex = 0;ElementIndex < NumElements;++ElementIndex)
        //     {
        //         if(!(A[ElementIndex] == B[ElementIndex]))
        //         {
        //             return true;
        //         }
        //     }
        //     return false;
        // }

        bool IsEmpty() const
        {
            return NumElements == 0;
        }

        int32 Num() const { return NumElements; }

        void Serialize(FArchive& Ar)
        {

        }

        void Deserialize(FArchive& Ar)
        {
            
        }

    private:
        struct alignas(Alignment) TArrayStorageElementAligned
        {
            TArrayStorageElementAligned() {}

            InElementType Element;
        };
        TArrayStorageElementAligned Elements[NumElements];
    };

	template <typename IteratorT>
	struct EnumerateIterator
	{
	public:
		using raw_reference = typename IteratorT::reference;
		using IdxValPair = std::pair<const size_t, raw_reference>;
		using iterator_category = std::forward_iterator_tag;
		using value_type = IdxValPair;
		using difference_type = ptrdiff_t;
		using pointer = IdxValPair*;
		using reference = IdxValPair&;

		explicit EnumerateIterator(IteratorT&& iterator) 
			: mCurIdx{ 0 }
			, mItr{ std::forward<IteratorT>(iterator) }
		{ }

		EnumerateIterator(IteratorT&& iterator, size_t startingCount)
			: mCurIdx{ startingCount }
			, mItr{ std::forward<IteratorT>(iterator) }
		{ }

		EnumerateIterator& operator++()
		{
			++mItr;
			++mCurIdx;
			return *this;
		}

		EnumerateIterator operator++(int)
		{
			auto temp{ *this };
			operator++();
			return temp;
		}

		bool operator==(const EnumerateIterator& enumItr) const
		{
			return (mCurIdx == enumItr.mCurIdx) && (mItr == enumItr.mItr);
		}

		bool operator!=(const EnumerateIterator& enumItr) const
		{
			return !(*this == enumItr);
		}

		IdxValPair operator*()
		{
			return IdxValPair(mCurIdx, *mItr);
		}

	private:
		size_t mCurIdx;
		IteratorT mItr;
	};

    template <typename T>
    struct EnemerateWrapper { T& Range; };

    template <typename T>
    EnemerateWrapper<T> Enumerate(T&& Range)
    {
        return EnemerateWrapper<T>{ Range };
    }

}

namespace std {
    template <typename T>
    auto begin(nilou::EnemerateWrapper<T> Wrapper)
    {
        return nilou::EnumerateIterator<decltype(std::begin(Wrapper.Range))>(std::begin(Wrapper.Range));
    }

    template <typename T>
    auto end(nilou::EnemerateWrapper<T> Wrapper)
    {
        return nilou::EnumerateIterator<decltype(std::end(Wrapper.Range))>(std::end(Wrapper.Range));
    }
}
