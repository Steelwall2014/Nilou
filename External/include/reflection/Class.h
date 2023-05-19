#pragma once
#include <queue>
#include <string>
#include <UDRefl/UDRefl.hpp>
#include <json/json.hpp>
#include "Macros.h"


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

    bool bIsSerializing = false;
};

inline NObject* CreateDefaultObject(const std::string& TypeName)
{
    auto Object = Ubpa::UDRefl::Mngr.New(Ubpa::Type(TypeName));
    auto BaseObject = Object.StaticCast(Ubpa::Type_of<NObject>);
    NObject* pObject = BaseObject.AsPtr<NObject>();
    if (pObject) 
        return pObject;
    Ubpa::UDRefl::Mngr.Delete(Object);
    return nullptr;
}

template<typename T>
class TStaticSerializer
{
    using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
public:
    static void Serialize(const RawT &Object, FArchive& Ar) 
    { 
        Ar.Node = Object;
    }
    static void Deserialize(RawT &Object, FArchive& Ar) 
    { 
        Object = Ar.Node.get<T>();
    }
};

template<typename T>
class TStaticSerializer<std::shared_ptr<T>>
{
    using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
public:
    static void Serialize(std::shared_ptr<RawT>& Object, FArchive& Ar) 
    { 
        if (Object)
            Object->Serialize(Ar);
    }
    static void Deserialize(std::shared_ptr<RawT>& Object, FArchive& Ar) 
    { 
        if (Ar.Node.contains("ClassName"))
        {
            std::string class_name = Ar.Node["ClassName"];
            if (Object == nullptr)
                Object = std::shared_ptr<T>(static_cast<T*>(CreateDefaultObject(class_name)));

            if (Object)
                Object->Deserialize(Ar);
        }
    }
};

template<typename T>
class TStaticSerializer<T*>
{
    using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
public:
    static void Serialize(RawT*& Object, FArchive& Ar) 
    { 
        if (Object)
            Object->Serialize(Ar);
    }
    static void Deserialize(RawT*& Object, FArchive& Ar) 
    { 
        if (Ar.Node.contains("ClassName"))
        {
            std::string class_name = Ar.Node["ClassName"];
            if (Object == nullptr)
                Object = static_cast<T*>(CreateDefaultObject(class_name));
            
            if (Object)
                Object->Deserialize(Ar);
        }
    }
};

template<>
class TStaticSerializer<FBinaryBuffer>
{
public:
    static void Serialize(const FBinaryBuffer &Object, FArchive& Ar) 
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