#pragma once

#include <unordered_set>

namespace nilou {

template<
    typename ElementType,
	typename KeyFuncs = std::hash<ElementType>,
    typename Allocator = std::allocator<ElementType>
    >
class TSet
{
private:
    using _MyBase = std::unordered_set<ElementType, KeyFuncs, std::equal_to<ElementType>, Allocator>;
public:
    using value_type = ElementType;
    using allocator_type = std::allocator<ElementType>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = ElementType&;
    using const_reference = const ElementType&;
    using pointer = ElementType*;
    using const_pointer = const ElementType*;
    using iterator = typename _MyBase::iterator;
    using const_iterator = typename _MyBase::const_iterator;

    // Adds an element to the set
    void Add(const ElementType& Element)
    {
        Set.insert(Element);
    }

    void Add(ElementType&& Element)
    {
        Set.insert(std::forward<ElementType>(Element));
    }

    // Removes an element from the set
    void Remove(const ElementType& Element)
    {
        Set.erase(Element);
    }

    // Checks if the set contains an element
    bool Contains(const ElementType& Element) const
    {
        return Set.find(Element) != Set.end();
    }

    // Returns the number of elements in the set
    size_t Num() const
    {
        return Set.size();
    }

    // Checks if the set is empty
    bool IsEmpty() const
    {
        return Set.empty();
    }

    void Empty()
    {
        Set.clear();
    }

    iterator begin() { return Set.begin(); }
    const_iterator begin() const { return Set.begin(); }
    const_iterator cbegin() const { return Set.cbegin(); }
    iterator end() { return Set.end(); }
    const_iterator end() const { return Set.end(); }
    const_iterator cend() const { return Set.cend(); }

private:
    _MyBase Set;
};

}
