#include "Class.h"
#include "base64.h"


void Serialize(FArchive& Ar, FBinaryBuffer& Value)
{
    static std::string Base64Header_OctetStream = "data:application/octet-stream;base64,";
    if (Ar.IsLoading())
    {
        std::string str = Ar.GetCurrentNode().get<std::string>();
        std::string_view Base64 = std::string_view(str).substr(Base64Header_OctetStream.size());
        std::string Base64Decoded = base64_decode(Base64.data(), Base64.size());
        Value.Buffer = std::make_shared<unsigned char[]>(Base64Decoded.size());
        Value.BufferSize = Base64Decoded.size();
        std::memcpy(Value.Buffer.get(), Base64Decoded.data(), Base64Decoded.size());
    }
    else
    {
        std::string Base64 = base64_encode(Value.Buffer.get(), Value.BufferSize);
        Ar.GetCurrentNode() = Base64Header_OctetStream + Base64;
    }
}

void Serialize(FArchive& Ar, NObject*& Object)
{
    Object->Serialize(Ar);
}
