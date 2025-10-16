#pragma once
#include <utility>
#include <iterator>
#include <vector>
#include "Platform.h"

namespace nilou {

    class FArchive;

    enum class EAllowShrinking : uint8
    {
        No,
        Yes
    };

    template<typename ElementType>
    class TArray
    {
    public:
        using value_type = ElementType;
        using allocator_type = std::allocator<ElementType>;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using reference = ElementType&;
        using const_reference = const ElementType&;
        using pointer = ElementType*;
        using const_pointer = const ElementType*;
        using iterator = typename std::vector<ElementType>::iterator;
        using const_iterator = typename std::vector<ElementType>::const_iterator;
        using reverse_iterator = typename std::vector<ElementType>::reverse_iterator;
        using const_reverse_iterator = typename std::vector<ElementType>::const_reverse_iterator;

        TArray() = default;
        TArray(std::initializer_list<ElementType> init) : Data(init) {}
        TArray(const TArray& Other) = default;
        TArray(TArray&& Other) noexcept = default;
        TArray& operator=(const TArray& Other) = default;
        TArray& operator=(TArray&& Other) noexcept = default;

        reference operator[](size_type index) noexcept { Ncheck(IsValidIndex(index)); return Data[index]; }
        const_reference operator[](size_type index) const noexcept { Ncheck(IsValidIndex(index)); return Data[index]; }
        reference At(size_type index) { return Data.at(index); }
        const_reference At(size_type index) const { return Data.at(index); }
        reference Front() { return Data.front(); }
        const_reference Front() const { return Data.front(); }
        reference Back() { return Data.back(); }
        const_reference Back() const { return Data.back(); }
        pointer GetData() { return Data.data(); }
        const_pointer GetData() const { return Data.data(); }

        bool IsEmpty() const { return Data.empty(); }
        size_type Num() const { return Data.size(); }
        size_type Max() const { return Data.capacity(); }
        void Reserve(size_type NewCapacity) { Data.reserve(NewCapacity); }
        void SetNum(size_type NewNum) { Data.resize(NewNum); }
        void Empty(size_type Slack = 0)
        {
            Data.clear();
            if (Slack > 0)
                Data.shrink_to_fit();
        }
        void Shrink() { Data.shrink_to_fit(); }

        void Add(const ElementType& Item) { Emplace(Item); }
        void Add(ElementType&& Item) { Emplace(std::move(Item)); }
        void AddUnique(const ElementType& Item)
        {
            if (!Contains(Item))
                Emplace(Item);
        }
        ElementType& AddDefaulted_GetRef()
        {
            return Emplace();
        }
        template<typename... Args>
        decltype(auto) Emplace(Args&&... args) { return Data.emplace_back(std::forward<Args>(args)...); }
        void Append(const TArray& Other)
        {
            Data.insert(Data.end(), Other.Data.begin(), Other.Data.end());
        }
        void Insert(const ElementType& Item, size_type Index)
        {
            Data.insert(Data.begin() + Index, Item);
        }
        void RemoveAt(size_type Index, size_type Count = 1, EAllowShrinking AllowShrinking = EAllowShrinking::Yes)
        {
            Data.erase(Data.begin() + Index, Data.begin() + Index + Count);
            if (AllowShrinking == EAllowShrinking::Yes)
                Data.shrink_to_fit();
        }
        bool Remove(const ElementType& Item)
        {
            auto it = std::remove(Data.begin(), Data.end(), Item);
            if (it != Data.end())
            {
                Data.erase(it, Data.end());
                return true;
            }
            return false;
        }
        ElementType Pop()
        {
            Ncheck(!Data.empty());
            ElementType Result = std::move(Data.back());
            Data.pop_back();
            return Result;
        }
        void Reset()
        {
            Data.clear();
        }

        int32 Find(const ElementType& Item) const
        {
            auto it = std::find(Data.begin(), Data.end(), Item);
            if (it != Data.end())
                return static_cast<int32>(std::distance(Data.begin(), it));
            return -1;
        }
        bool Contains(const ElementType& Item) const
        {
            return std::find(Data.begin(), Data.end(), Item) != Data.end();
        }
        bool IsValidIndex(size_type Index) const
        {
            return Index < Data.size();
        }

        iterator begin() { return Data.begin(); }
        const_iterator begin() const { return Data.begin(); }
        const_iterator cbegin() const { return Data.cbegin(); }
        iterator end() { return Data.end(); }
        const_iterator end() const { return Data.end(); }
        const_iterator cend() const { return Data.cend(); }
        reverse_iterator rbegin() { return Data.rbegin(); }
        const_reverse_iterator rbegin() const { return Data.rbegin(); }
        reverse_iterator rend() { return Data.rend(); }
        const_reverse_iterator rend() const { return Data.rend(); }

        void Swap(TArray& Other)
        {
            Data.swap(Other.Data);
        }

        friend bool operator==(const TArray& A, const TArray& B)
        {
            return A.Data == B.Data;
        }
        friend bool operator!=(const TArray& A, const TArray& B)
        {
            return !(A == B);
        }

        std::vector<ElementType>& InternalGetStdVector() { return Data; }
        const std::vector<ElementType>& InternalGetStdVector() const { return Data; }

        template<typename T>
        friend void Serialize(FArchive& Ar, TArray<T>& Array);

    private:
        std::vector<ElementType> Data;
    };

    template<typename ElementType>
    class TArrayView
    {
    public:
        using value_type = ElementType;
        using pointer = ElementType*;
        using reference = ElementType&;
        using const_pointer = const ElementType*;
        using const_reference = const ElementType&;
        using iterator = ElementType*;
        using const_iterator = const ElementType*;
        using size_type = size_t;

        TArrayView()
            : DataPtr(nullptr), ArrayNum(0)
        {
        }

        TArrayView(ElementType* InData, size_t InNum)
            : DataPtr(InData), ArrayNum(InNum)
        {
        }

        TArrayView(std::vector<ElementType>& Vec)
            : DataPtr(Vec.data()), ArrayNum(Vec.size())
        {
        }

        TArrayView(const std::vector<ElementType>& Vec)
            : DataPtr(Vec.data()), ArrayNum(Vec.size())
        {
        }

        TArrayView(class TArray<ElementType>& Arr)
            : DataPtr(Arr.Num() > 0 ? &Arr[0] : nullptr), ArrayNum(Arr.Num())
        {
        }

        TArrayView(const class TArray<ElementType>& Arr)
            : DataPtr(Arr.Num() > 0 ? &Arr[0] : nullptr), ArrayNum(Arr.Num())
        {
        }

        iterator begin() { return DataPtr; }
        iterator end() { return DataPtr + ArrayNum; }
        const_iterator begin() const { return DataPtr; }
        const_iterator end() const { return DataPtr + ArrayNum; }
        const_iterator cbegin() const { return DataPtr; }
        const_iterator cend() const { return DataPtr + ArrayNum; }

        size_t Num() const { return ArrayNum; }
        bool IsEmpty() const { return ArrayNum == 0; }

        reference operator[](size_t Index)
        {
            return DataPtr[Index];
        }

        const_reference operator[](size_t Index) const
        {
            return DataPtr[Index];
        }

        pointer GetData() { return DataPtr; }
        const_pointer GetData() const { return DataPtr; }

    private:
        ElementType* DataPtr;
        size_t ArrayNum;
    };

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
			return mItr == enumItr.mItr;
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
