#include <ctime>
#include <map>
#include <unordered_map>
#include <iostream>
#include <array>


template <typename IteratorT>
struct EnumerateIterator
{
public:
    using raw_value_type = typename IteratorT::value_type;
    using IdxValPair = std::pair<const size_t, raw_value_type&>;
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


namespace std {
    template <typename T>
    auto begin(EnemerateWrapper<T> Wrapper)
    {
        return EnumerateIterator<decltype(std::begin(Wrapper.Range))>(std::begin(Wrapper.Range));
    }

    template <typename T>
    auto end(EnemerateWrapper<T> Wrapper)
    {
        return EnumerateIterator<decltype(std::end(Wrapper.Range))>(std::end(Wrapper.Range));
    }
}

int main()
{
    std::array<int, 8> a = { 0 };
    for (auto [index, x] : Enumerate(a))
    {
        x = index;
    }
    return 0;
}