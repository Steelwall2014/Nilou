#pragma once
#include <list>
#include <unordered_map>
#include <optional>

namespace nilou {

    template<typename Key, typename Value>
    class TLruCache
    {
    public:

        TLruCache(int InCapacity)
            : Capacity(InCapacity)
        { }

        Value& Get(const Key& key)
        {
            if (ItemToIterMap.find(key) != ItemToIterMap.end())
            {
                std::list<std::pair<Key, Value>>::iterator iter = ItemToIterMap[key];
                MoveToFront(iter);
                return *iter;
            }
        }

        std::optional<std::pair<Key, Value>> Put(const Key& key, const Value& value)
        {
            if (ItemToIterMap.find(key) != ItemToIterMap.end())
            {
                std::list<std::pair<Key, Value>>::iterator iter = ItemToIterMap[key];
                *iter = std::make_pair(key, value);
                MoveToFront(iter);
            }
            else 
            {
                std::list<std::pair<Key, Value>>::iterator iter = ItemList.insert(ItemList.begin(), std::make_pair(key, value));
                ItemToIterMap[key] = iter;
                if (ItemList.size() > Capacity)
                {
                    std::list<std::pair<Key, Value>>::iterator IterToRemove = ItemList.end();
                    IterToRemove--;
                    auto &[key, value] = *IterToRemove;
                    ItemToIterMap.erase(key);
                    ItemList.erase(IterToRemove);
                    return std::make_pair(key, value);
                }
            }
            return std::nullopt;
        }

        int GetCapacity() const 
        { 
            return Capacity; 
        }

        void SetCapacity(int InCapacity)
        {
            Capacity = InCapacity;
        }

    private:

        int Capacity;
        std::list<std::pair<Key, Value>> ItemList;
        std::unordered_map<Key, std::list<std::pair<Key, Value>>::iterator> ItemToIterMap;

        void MoveToFront(std::list<std::pair<Key, Value>>::iterator iter)
        {
            if (iter != ItemList.begin())
                ItemList.splice(ItemList.begin(), ItemList, iter, std::next(iter));
        }
    };

}