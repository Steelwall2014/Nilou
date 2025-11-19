#pragma once
#include <string>

#include <nlohmann/json.hpp>
#include "Common/Containers/Array.h"
#include "Common/Containers/Map.h"
#include "Platform.h"

namespace nilou {

class FArchive
{
public:

    FArchive(nlohmann::json& InNode, bool bInIsLoading = false) noexcept
        : Node(InNode)
        , bIsLoading(bInIsLoading)
    { }

    FArchive(FArchive&& Other) noexcept
        : Node(Other.Node)
        , bIsLoading(Other.bIsLoading)
    { }

    bool IsLoading() const
    {
        return bIsLoading;
    }

    FArchive& operator[](const std::string& Key);

    FArchive& operator[](size_t Index);

    nlohmann::json& GetNode()
    {
        return Node;
    }

private:
    nlohmann::json& Node;
    bool bIsLoading;

    TArray<std::unique_ptr<FArchive>> ArrayChildren;
    TMap<std::string, std::unique_ptr<FArchive>> ObjectChildren;

};

template<typename T>
void Serialize(FArchive& Ar, TArray<T>& Array)
{
    nlohmann::json& Node = Ar.GetNode();
    if (Ar.IsLoading())
    {
        Ncheck(Node.is_array());
        Array.SetNum(Node.size());
        for (int32 i = 0; i < Node.size(); ++i)
        {
            Serialize(Ar[i], Array[i]);
        }
    }
    else 
    {
        Node = nlohmann::json::array();
        for (int32 i = 0; i < Array.Num(); ++i)
        {
            Serialize(Ar[i], Array[i]);
        }
    }
}

template<typename TKey, typename TValue>
void Serialize(FArchive& Ar, TMap<TKey, TValue>& Map)
{
    nlohmann::json& Node = Ar.GetNode();
    if (Ar.IsLoading())
    {
        Ncheck(Node.is_array());
        Map.Empty();
        for (int32 i = 0; i < Node.size(); ++i)
        {
            TKey Key = TKey();
            Serialize(Ar[i]["Key"], Key);
            TValue Value = TValue();
            Serialize(Ar[i]["Value"], Value);
            Map.Add(Key, Value);
        }
    }
    else 
    {
        Node = nlohmann::json::array();
        int32 i = 0;
        for (auto& Pair : Map)
        {
            Serialize(Ar[i]["Key"], Pair.Key);
            Serialize(Ar[i]["Value"], Pair.Value);
            i++;
        }
    }
}

}
