#pragma once
#include <list>
#include <unordered_map>
#include <optional>
#include <mutex>

namespace nilou {

    template<typename Key, typename Value>
    class TLruCache
    {
        using TIter = typename std::list<std::pair<Key, Value>>::iterator;
    public:

        TLruCache(int InCapacity)
            : Capacity(InCapacity)
        { }

        Value& Get(const Key& key)
        {
            if (ItemToIterMap.find(key) != ItemToIterMap.end())
            {
                TIter iter = ItemToIterMap[key];
                MoveToFront(iter);
                return *iter;
            }
        }

        Value& Get_ThreadSafe(const Key& key)
        {
            std::lock_guard<std::mutex> lock(mutex);
            return Get(key);
        }

        std::optional<std::pair<Key, Value>> Put(const Key& key, const Value& value)
        {
            if (ItemToIterMap.find(key) != ItemToIterMap.end())
            {
                TIter iter = ItemToIterMap[key];
                *iter = std::make_pair(key, value);
                MoveToFront(iter);
            }
            else 
            {
                TIter iter = ItemList.insert(ItemList.begin(), std::make_pair(key, value));
                ItemToIterMap[key] = iter;
                if (ItemList.size() > Capacity)
                {
                    TIter IterToRemove = ItemList.end();
                    IterToRemove--;
                    auto &[key, value] = *IterToRemove;
                    ItemToIterMap.erase(key);
                    ItemList.erase(IterToRemove);
                    return std::make_pair(key, value);
                }
            }
            return std::nullopt;
        }

        std::optional<std::pair<Key, Value>> Put_ThreadSafe(const Key& key, const Value& value)
        {
            std::lock_guard<std::mutex> lock(mutex);
            return Put(key, value);
        }

        int GetCapacity() const 
        { 
            return Capacity; 
        }

        void SetCapacity(int InCapacity)
        {
            std::lock_guard<std::mutex> lock(mutex);
            Capacity = InCapacity;
        }

    private:

        int Capacity;
        std::list<std::pair<Key, Value>> ItemList;
        std::unordered_map<Key, TIter> ItemToIterMap;
        std::mutex mutex;

        void MoveToFront(TIter iter)
        {
            if (iter != ItemList.begin())
                ItemList.splice(ItemList.begin(), ItemList, iter, std::next(iter));
        }
    };

}