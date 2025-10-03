#pragma once
#include <unordered_map>

namespace nilou {

    class FArchive;
    
    template<typename KeyType, typename ValueType>
    class TMap
    {
    public:
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<const KeyType, ValueType>;
        using iterator = typename std::unordered_map<KeyType, ValueType>::iterator;
        using const_iterator = typename std::unordered_map<KeyType, ValueType>::const_iterator;
        using size_type = typename std::unordered_map<KeyType, ValueType>::size_type;

        TMap() = default;
        TMap(std::initializer_list<value_type> Init) : Data(Init) {}
        TMap(const TMap& Other) = default;
        TMap(TMap&& Other) noexcept = default;
        TMap& operator=(const TMap& Other) = default;
        TMap& operator=(TMap&& Other) noexcept = default;

        ValueType& operator[](const KeyType& Key)
        {
            return Data[Key];
        }
        
        ValueType* Find(const KeyType& Key)
        {
            auto it = Data.find(Key);
            return it != Data.end() ? &it->second : nullptr;
        }

        const ValueType* Find(const KeyType& Key) const
        {
            auto it = Data.find(Key);
            return it != Data.end() ? &it->second : nullptr;
        }

        ValueType FindRef(const KeyType& Key) const
        {
            auto it = Data.find(Key);
            return it != Data.end() ? it->second : ValueType();
        }

        ValueType& FindOrAdd(const KeyType& Key)
        {
            return Data[Key];
        }

        bool Contains(const KeyType& Key) const
        {
            return Data.find(Key) != Data.end();
        }

        template <typename _KeyType, typename _ValueType>
        void Add(_KeyType&& Key, _ValueType&& Value)
        {
            Data[std::forward<_KeyType>(Key)] = std::forward<_ValueType>(Value);
        }

        void Add(const value_type& Pair)
        {
            Data[Pair.first] = Pair.second;
        }

        bool Remove(const KeyType& Key)
        {
            return Data.erase(Key) > 0;
        }

        void Empty()
        {
            Data.clear();
        }

        size_type Num() const
        {
            return Data.size();
        }

        bool IsEmpty() const
        {
            return Data.empty();
        }

        iterator begin() { return Data.begin(); }
        iterator end() { return Data.end(); }
        const_iterator begin() const { return Data.begin(); }
        const_iterator end() const { return Data.end(); }
        const_iterator cbegin() const { return Data.cbegin(); }
        const_iterator cend() const { return Data.cend(); }

        void Reserve(size_type NewCapacity)
        {
            Data.reserve(NewCapacity);
        }

        void Append(const TMap& Other)
        {
            for (const auto& Pair : Other)
            {
                Data[Pair.first] = Pair.second;
            }
        }

        void Reset()
        {
            Data.clear();
        }

        void Clear()
        {
            Data.clear();
        }

        std::unordered_map<KeyType, ValueType>& GetStdMap()
        {
            return Data;
        }

        const std::unordered_map<KeyType, ValueType>& GetStdMap() const
        {
            return Data;
        }

    private:
        std::unordered_map<KeyType, ValueType> Data;

        template<typename TKey, typename TValue>
        friend void Serialize(FArchive& Ar, TMap<TKey, TValue>& Map);
    };

}
