#pragma once
#include <queue>
#include <string>
#include <UDRefl/UDRefl.hpp>
#include <json/json.hpp>
#include "Macros.h"
#include <magic_enum.hpp>


template<typename T>
struct TClassRegistry { };

class NClass
{
public:

    template<typename T>
    friend class TClassRegistry;

    NClass() = default;

    bool IsChildOf(const NClass *BaseClass) const
    {
        if (Type == BaseClass->Type)
            return true;
        std::queue<Ubpa::Type> q;
        q.push(Type);
        while (!q.empty())
        {
            Ubpa::Type temp_class = q.front(); q.pop();
            auto temp_info = Ubpa::UDRefl::Mngr.GetTypeInfo(temp_class);
            for (auto& [parent_class, base_info] : temp_info->baseinfos)
            {
                if (parent_class == BaseClass->Type)
                    return true;
                q.push(parent_class);
            }
        }
        return false;
    }

    inline bool operator==(const NClass &Other) const
    {
        return Type == Other.Type;
    }

    inline bool operator<(const NClass &Other) const
    {
        return Type < Other.Type;
    }

    Ubpa::Type GetType() const
    {
        return Type;
    }

    const Ubpa::UDRefl::TypeInfo* GetTypeInfo() const
    {
        return TypeInfo;
    }

private:

    const Ubpa::UDRefl::TypeInfo *TypeInfo;

    Ubpa::Type Type;

};

struct FBinaryBuffer
{
    unsigned int BufferSize;

    std::shared_ptr<unsigned char[]> Buffer;
};

struct FArchiveBuffer
{
    FArchiveBuffer(nlohmann::json &InCurrentNode, FBinaryBuffer InBuffer)
        : Node(InCurrentNode), Buffer(InBuffer)
    { }
    FArchiveBuffer(const FArchiveBuffer& Other)
        : Node(Other.Node)
        , Buffer(Other.Buffer)
    { }
    FArchiveBuffer& operator=(const FArchiveBuffer& Other)
    { 
        Node = Other.Node;
        Buffer = Other.Buffer;
        return *this;
    }
    nlohmann::json &Node;
    FBinaryBuffer Buffer;
};

class FArchive
{
public:
    FArchive(nlohmann::json& CurrentNode, const FArchive& Other)
        : Node(CurrentNode) 
        , OutBuffers(Other.OutBuffers)
        , Version(Other.Version)
        , FileLength(Other.FileLength)
        , JsonLength(Other.JsonLength)
        , BinLength(Other.BinLength)
        , InBuffer(Other.InBuffer)
    { }
    FArchive(nlohmann::json& CurrentNode, std::vector<FArchiveBuffer>& OutBuffers)
        : Node(CurrentNode) 
        , OutBuffers(OutBuffers)
    { }
    FArchive(const FArchive& Other)
        : FArchive(Other.Node, Other)
    { }
    FArchive& operator=(const FArchive& Other)
    {
        Node = Other.Node;
        OutBuffers = Other.OutBuffers;
        Version = Other.Version;
        FileLength = Other.FileLength;
        JsonLength = Other.JsonLength;
        BinLength = Other.BinLength;
        InBuffer = Other.InBuffer;

        return *this;
    }
    nlohmann::json& Node;
    std::vector<FArchiveBuffer>& OutBuffers;
    unsigned int Version;
    unsigned int FileLength;
    unsigned int JsonLength;
    unsigned int BinLength;
    std::shared_ptr<unsigned char[]> InBuffer = nullptr;

    friend std::ostream& operator<<(std::ostream& out, FArchive& Ar)
    {
        Ar.BinLength = 0;
        for (int i = 0; i < Ar.OutBuffers.size(); i++)
        {
            auto &Block = Ar.OutBuffers[i];
            Block.Node["BufferOffset"] = Ar.BinLength;
            Ar.BinLength += Block.Buffer.BufferSize;
        }
        std::string json_str = Ar.Node.dump();
        Ar.JsonLength = json_str.size();
        Ar.FileLength = Ar.BinLength + Ar.JsonLength + 
                    4 + // magic
                    4 + // version
                    4 + // length
                    4 + // json chunk length
                    4 + // json chunk type ('J', 'S', 'O', 'N')
                    4 + // binary chunk length
                    4;  // binary chunk type ('B', 'I', 'N', '\0')
        char magic[4];
        magic[0] = {'n'};
        magic[1] = {'a'};
        magic[2] = {'s'};
        magic[3] = {'t'};
        out.write(magic, 4);
        Ar.Version = 1;
        out.write((char*)&Ar.Version, 4);
        out.write((char*)&Ar.FileLength, 4);

        out.write((char*)&Ar.JsonLength, 4);
        char JsonChunkType[4];
        JsonChunkType[0] = {'J'};
        JsonChunkType[1] = {'S'};
        JsonChunkType[2] = {'O'};
        JsonChunkType[3] = {'N'};
        out.write(JsonChunkType, 4);
        out.write(json_str.c_str(), json_str.size());

        out.write((char*)&Ar.BinLength, 4);
        char BinChunkType[4];
        BinChunkType[0] = {'B'};
        BinChunkType[1] = {'I'};
        BinChunkType[2] = {'N'};
        BinChunkType[3] = {'\0'};
        out.write(BinChunkType, 4);
        for (int i = 0; i < Ar.OutBuffers.size(); i++)
        {
            auto &Block = Ar.OutBuffers[i];
            out.write((char*)Block.Buffer.Buffer.get(), Block.Buffer.BufferSize);
        }
        return out;
    }

    friend std::istream& operator>>(std::istream& in, FArchive& Ar)
    {
        char magic[4];
        in.read(magic, 4);
        if (magic[0] != 'n' || 
            magic[1] != 'a' || 
            magic[2] != 's' || 
            magic[3] != 't')
            return in;
        
        in.read((char*)&Ar.Version, 4);
        
        in.read((char*)&Ar.FileLength, 4);

        
        char JsonChunkType[4];
        in.read((char*)&Ar.JsonLength, 4);
        in.read(JsonChunkType, 4);
        std::unique_ptr<char[]> json = std::make_unique<char[]>(Ar.JsonLength+1);
        in.read(json.get(), Ar.JsonLength);
        json[Ar.JsonLength] = '\0';
        std::stringstream(json.get()) >> Ar.Node;

        char BinChunkType[4];
        in.read((char*)&Ar.BinLength, 4);
        in.read(BinChunkType, 4);
        Ar.InBuffer = std::make_unique<unsigned char[]>(Ar.BinLength);
        in.read((char*)Ar.InBuffer.get(), Ar.BinLength);

        return in;
    }

};

class NCLASS NObject
{
    GENERATED_BODY();
public:
    NFUNCTION()
    bool IsA(const NClass *Class)
    {
        return GetClass()->IsChildOf(Class);
    }

    NFUNCTION()
    std::string_view GetClassName() const
    {
        return GetClass()->GetType().GetName();
    }

    virtual void PostDeserialize() { }

    bool bIsSerializing = false;
};

inline NObject* CreateDefaultObject(const std::string& TypeName)
{
    auto Object = Ubpa::UDRefl::Mngr.New(Ubpa::Type(TypeName));
    auto BaseObject = Object.StaticCast(Ubpa::Type_of<NObject>);
    NObject* pObject = BaseObject.AsPtr<NObject>();
    if (pObject) 
        return pObject;
    if (Object)
        Ubpa::UDRefl::Mngr.Delete(Object);
    return nullptr;
}

/** Older solution */
// template<typename, typename T>
// struct HasMethodSerialize
// {
//     static_assert(
//         std::integral_constant<T, false>::value,
//         "Second template parameter needs to be of function type.");
// };

// template<typename T, typename TReturn, typename ...TArgs>
// struct HasMethodSerialize<T, TReturn(TArgs...)>
// {
// private:
//     template<typename U>
//     static constexpr auto Check(int) 
//         -> typename std::is_same<decltype(std::declval<U>().Serialize(std::declval<TArgs>()...)), TReturn>::type;
//     template<typename U>
//     static constexpr std::false_type Check(...);
//     typedef decltype(Check<T>(0)) type;
// public:
//     static constexpr bool value = std::is_same<type, std::true_type>::value;
// };

// template<typename, typename T>
// struct HasMethodDeserialize
// {
//     static_assert(
//         std::integral_constant<T, false>::value,
//         "Second template parameter needs to be of function type.");
// };

// template<typename T, typename TReturn, typename ...TArgs>
// struct HasMethodDeserialize<T, TReturn(TArgs...)>
// {
// private:
//     template<typename U>
//     static constexpr auto Check(int) 
//         -> typename std::is_same<decltype(std::declval<U>().Deserialize(std::declval<TArgs>()...)), TReturn>::type;
//     template<typename U>
//     static constexpr std::false_type Check(...);
//     typedef decltype(Check<T>(0)) type;
// public:
//     static constexpr bool value = std::is_same<type, std::true_type>::value;
// };

/** C++ 20 solution */
template<typename T, typename TReturn, typename ...TArgs>
concept HasMethodSerialize = requires {
    { std::declval<T>().Serialize(std::declval<TArgs>()...) } -> std::same_as<TReturn>;
};

template<typename T, typename TReturn, typename ...TArgs>
concept HasMethodDeserialize = requires {
    { std::declval<T>().Deserialize(std::declval<TArgs>()...) } -> std::same_as<TReturn>;
};

template<class T>
concept IsMapType = std::same_as<typename T::value_type, std::pair<const typename T::key_type, typename T::mapped_type>>;

template<class T>
concept IsSetType = std::same_as<typename T::value_type, typename T::key_type>;

template<class T>
concept IsRangeContainerType = requires {
    { std::declval<T>().data() } -> std::same_as<typename T::value_type *>;
};

template<class T>
concept HasMethodResize = requires {
    { std::declval<T>().resize(1) } -> std::same_as<void>;
};

template<typename T>
class TStaticSerializer
{
    using RawT = std::remove_cvref_t<T>;
public:
    static void Serialize(RawT &Object, FArchive& Ar) 
    { 
        if constexpr (std::is_enum_v<RawT>)
            Ar.Node = magic_enum::enum_name(Object);
        else if constexpr (std::is_same_v<T, std::string>)
            Ar.Node = Object;
        else if constexpr (HasMethodSerialize<RawT, void, FArchive&>)
            Object.Serialize(Ar);
        else if constexpr (IsMapType<T>)
        {
            for (auto &[ckey, value] : Object)
            {
                typename T::key_type key = ckey;
                nlohmann::json& Node = Ar.Node.emplace_back();
                FArchive local_Ar_key(Node["key"], Ar);
                TStaticSerializer<typename T::key_type>::Serialize(key, local_Ar_key);
                FArchive local_Ar_value(Node["value"], Ar);
                TStaticSerializer<typename T::mapped_type>::Serialize(value, local_Ar_value);
            }
        }
        else if constexpr (IsRangeContainerType<T>)
        {
            for (int i = 0; i < Object.size(); i++)
            {
                nlohmann::json& Node = Ar.Node.emplace_back();
                FArchive local_Ar(Node, Ar);
                TStaticSerializer<typename T::value_type>::Serialize(Object[i], local_Ar);
            }
        }
        else if constexpr (IsSetType<T>)
        {
            for (auto& cvalue : Object)
            {
                using TValue = typename T::value_type;
                TValue value = cvalue;
                nlohmann::json& Node = Ar.Node.emplace_back();
                FArchive local_Ar(Node, Ar);
                TStaticSerializer<TValue>::Serialize(value, local_Ar);
            }
        }
        else 
        {
            Ar.Node = Object;
        }
    }
    static void Deserialize(RawT &Object, FArchive& Ar) 
    { 
        if constexpr (std::is_enum_v<RawT>)
        {
            auto opt = magic_enum::enum_cast<RawT>(Ar.Node.get<std::string>());
            if (opt)
                Object = opt.value();
        }
        else if constexpr (std::is_same_v<T, std::string>)
            Object = Ar.Node.get<RawT>();
        else if constexpr (HasMethodDeserialize<RawT, void, FArchive&>)
            Object.Deserialize(Ar);
        else if constexpr (IsMapType<T>)
        {
            if (Ar.Node.is_array())
            {
                for (int i = 0; i < Ar.Node.size(); i++)
                {
                    nlohmann::json& Node = Ar.Node[i];
                    if (Node.contains("key") && Node.contains("value"))
                    {
                        using TKey = typename T::key_type;
                        using TValue = typename T::mapped_type;
                        std::unique_ptr<TKey> key = std::make_unique<TKey>();
                        std::unique_ptr<TValue> value = std::make_unique<TValue>();
                        FArchive local_Ar_key(Node["key"], Ar);
                        TStaticSerializer<TKey>::Deserialize(*key, local_Ar_key);
                        FArchive local_Ar_value(Node["value"], Ar);
                        TStaticSerializer<TValue>::Deserialize(*value, local_Ar_value);
                        Object[*key] = *value;
                    }
                }
            }
        }
        else if constexpr (IsRangeContainerType<T>)
        {
            if (Ar.Node.is_array())
            {
                if constexpr (HasMethodResize<T>)
                    Object.resize(Ar.Node.size());
                for (int i = 0; i < Object.size(); i++)
                {
                    FArchive local_Ar(Ar.Node[i], Ar);
                    TStaticSerializer<typename T::value_type>::Deserialize(Object[i], local_Ar);
                }
            }
        }
        else if constexpr (IsSetType<T>)
        {
            if (Ar.Node.is_array())
            {
                for (int i = 0; i < Ar.Node.size(); i++)
                {
                    using TKey = typename T::key_type;
                    nlohmann::json& Node = Ar.Node[i];
                    std::unique_ptr<TKey> pkey = std::make_unique<TKey>();
                    FArchive local_Ar(Node, Ar);
                    TStaticSerializer<TKey>::Deserialize(*pkey, local_Ar);
                    Object.insert(*pkey);
                }
            }
        }
        else
        {
            Object = Ar.Node.get<RawT>();
        }
    }
};

template<>
class TStaticSerializer<FBinaryBuffer>
{
public:
    static void Serialize(FBinaryBuffer &Object, FArchive& Ar) 
    { 
        if (Object.BufferSize && Object.Buffer)
        {
            Ar.Node["BufferSize"] = Object.BufferSize;
            Ar.OutBuffers.push_back(FArchiveBuffer(Ar.Node, Object));
        }
    }
    static void Deserialize(FBinaryBuffer &Object, FArchive& Ar) 
    { 
        if (Ar.Node.contains("BufferOffset") && Ar.Node.contains("BufferSize"))
        {
            size_t BufferOffset = Ar.Node["BufferOffset"].get<size_t>();
            Object.BufferSize = Ar.Node["BufferSize"].get<size_t>();
            Object.Buffer = std::make_shared<unsigned char[]>(Object.BufferSize);
            std::copy(
                Ar.InBuffer.get()+BufferOffset, 
                Ar.InBuffer.get()+BufferOffset+Object.BufferSize, 
                Object.Buffer.get());
        }
    }
};