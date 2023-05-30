#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FVertexIndexBufferData::StaticClass_ = nullptr;
const NClass *nilou::FVertexIndexBufferData::GetClass() const 
{ 
    return nilou::FVertexIndexBufferData::StaticClass(); 
}
const NClass *nilou::FVertexIndexBufferData::StaticClass()
{
    return nilou::FVertexIndexBufferData::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FVertexIndexBufferData>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FVertexIndexBufferData::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FVertexIndexBufferData>();
		Mngr.AddField<&nilou::FVertexIndexBufferData::Data>("Data");
		Mngr.AddField<&nilou::FVertexIndexBufferData::Stride>("Stride");
		Mngr.AddMethod<&nilou::FVertexIndexBufferData::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FVertexIndexBufferData::Serialize>("Serialize");
;
        nilou::FVertexIndexBufferData::StaticClass_->Type = Type_of<nilou::FVertexIndexBufferData>;
        nilou::FVertexIndexBufferData::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FVertexIndexBufferData>);
    }

    static TClassRegistry<nilou::FVertexIndexBufferData> Dummy;
};
TClassRegistry<nilou::FVertexIndexBufferData> Dummy = TClassRegistry<nilou::FVertexIndexBufferData>("nilou::FVertexIndexBufferData");



void nilou::FVertexIndexBufferData::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Data"], Ar);
        TStaticSerializer<decltype(this->Data)>::Serialize(this->Data, local_Ar);
    }
    {
        FArchive local_Ar(content["Stride"], Ar);
        TStaticSerializer<decltype(this->Stride)>::Serialize(this->Stride, local_Ar);
    }
}

void nilou::FVertexIndexBufferData::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Data"))
    {
        FArchive local_Ar(content["Data"], Ar);
        TStaticSerializer<decltype(this->Data)>::Deserialize(this->Data, local_Ar);
    }
    if (content.contains("Stride"))
    {
        FArchive local_Ar(content["Stride"], Ar);
        TStaticSerializer<decltype(this->Stride)>::Deserialize(this->Stride, local_Ar);
    }
    
}
