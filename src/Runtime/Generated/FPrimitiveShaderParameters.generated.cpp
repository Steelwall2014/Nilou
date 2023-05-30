#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FPrimitiveShaderParameters::StaticClass_ = nullptr;
const NClass *nilou::FPrimitiveShaderParameters::GetClass() const 
{ 
    return nilou::FPrimitiveShaderParameters::StaticClass(); 
}
const NClass *nilou::FPrimitiveShaderParameters::StaticClass()
{
    return nilou::FPrimitiveShaderParameters::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FPrimitiveShaderParameters>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FPrimitiveShaderParameters::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FPrimitiveShaderParameters>();
		Mngr.AddField<&nilou::FPrimitiveShaderParameters::LocalToWorld>("LocalToWorld");
		Mngr.AddField<&nilou::FPrimitiveShaderParameters::ModelToLocal>("ModelToLocal");
		Mngr.AddMethod<&nilou::FPrimitiveShaderParameters::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FPrimitiveShaderParameters::Serialize>("Serialize");
;
        nilou::FPrimitiveShaderParameters::StaticClass_->Type = Type_of<nilou::FPrimitiveShaderParameters>;
        nilou::FPrimitiveShaderParameters::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FPrimitiveShaderParameters>);
    }

    static TClassRegistry<nilou::FPrimitiveShaderParameters> Dummy;
};
TClassRegistry<nilou::FPrimitiveShaderParameters> Dummy = TClassRegistry<nilou::FPrimitiveShaderParameters>("nilou::FPrimitiveShaderParameters");



void nilou::FPrimitiveShaderParameters::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["LocalToWorld"], Ar);
        TStaticSerializer<decltype(this->LocalToWorld)>::Serialize(this->LocalToWorld, local_Ar);
    }
    {
        FArchive local_Ar(content["ModelToLocal"], Ar);
        TStaticSerializer<decltype(this->ModelToLocal)>::Serialize(this->ModelToLocal, local_Ar);
    }
}

void nilou::FPrimitiveShaderParameters::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("LocalToWorld"))
    {
        FArchive local_Ar(content["LocalToWorld"], Ar);
        TStaticSerializer<decltype(this->LocalToWorld)>::Deserialize(this->LocalToWorld, local_Ar);
    }
    if (content.contains("ModelToLocal"))
    {
        FArchive local_Ar(content["ModelToLocal"], Ar);
        TStaticSerializer<decltype(this->ModelToLocal)>::Deserialize(this->ModelToLocal, local_Ar);
    }
    
}
