#pragma once
#include <string>

#include <json/json.hpp>
#include "Common/Containers/Array.h"
#include "Common/Containers/Map.h"
#include "Platform.h"

namespace nilou {

class FArchive
{
public:

    FArchive(nlohmann::json& InNode, bool bInIsLoading = false)
        : Node(InNode)
        , bIsLoading(bInIsLoading)
    { }

    FArchive(FArchive&& Other)
        : Node(Other.Node)
        , bIsLoading(Other.bIsLoading)
    { }

    bool IsLoading() const
    {
        return bIsLoading;
    }

    FArchive& operator[](const std::string& Key)
    {
        if (!ObjectChildren.Contains(Key))
        {
            nlohmann::json& ChildNode = Node[Key];
            ObjectChildren.Add(Key, FArchive(ChildNode, bIsLoading));
        }
        return ObjectChildren[Key];
    }

    FArchive& operator[](size_t Index)
    {
        if (!ArrayChildren.Contains(Index))
        {
            nlohmann::json& ChildNode = Node[Index];
            ArrayChildren.Add(Index, FArchive(ChildNode, bIsLoading));
        }
        return ArrayChildren[Index];
    }

    nlohmann::json& GetNode()
    {
        return Node;
    }

private:
    nlohmann::json& Node;
    bool bIsLoading;

    TMap<size_t, FArchive> ArrayChildren;
    TMap<std::string, FArchive> ObjectChildren;

};

template<typename T>
void Serialize(FArchive& Ar, TArray<T>& Array)
{
    nlohmann::json& Node = Ar.GetNode();
    if (Ar.IsLoading())
    {
        Array.SetNum(Node.size());
        for (int32 i = 0; i < Node.size(); ++i)
        {
            Serialize(Ar[i], Array[i]);
        }
    }
    else 
    {
        for (int32 i = 0; i < Array.Num(); ++i)
        {
            Node.emplace_back();
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
        int32 i = 0;
        for (auto& Pair : Map)
        {
            Node.emplace_back();
            Serialize(Ar[i]["Key"], Pair.Key);
            Serialize(Ar[i]["Value"], Pair.Value);
            i++;
        }
    }
}

}
