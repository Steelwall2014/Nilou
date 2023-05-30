#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FShadowMappingParameters::StaticClass_ = nullptr;
const NClass *nilou::FShadowMappingParameters::GetClass() const 
{ 
    return nilou::FShadowMappingParameters::StaticClass(); 
}
const NClass *nilou::FShadowMappingParameters::StaticClass()
{
    return nilou::FShadowMappingParameters::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FShadowMappingParameters>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FShadowMappingParameters::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FShadowMappingParameters>();
		Mngr.AddField<&nilou::FShadowMappingParameters::FrustumFar>("FrustumFar");
		Mngr.AddField<&nilou::FShadowMappingParameters::Resolution>("Resolution");
		Mngr.AddField<&nilou::FShadowMappingParameters::WorldToClip>("WorldToClip");
		Mngr.AddMethod<&nilou::FShadowMappingParameters::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FShadowMappingParameters::Serialize>("Serialize");
;
        nilou::FShadowMappingParameters::StaticClass_->Type = Type_of<nilou::FShadowMappingParameters>;
        nilou::FShadowMappingParameters::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FShadowMappingParameters>);
    }

    static TClassRegistry<nilou::FShadowMappingParameters> Dummy;
};
TClassRegistry<nilou::FShadowMappingParameters> Dummy = TClassRegistry<nilou::FShadowMappingParameters>("nilou::FShadowMappingParameters");



void nilou::FShadowMappingParameters::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["FrustumFar"], Ar);
        TStaticSerializer<decltype(this->FrustumFar)>::Serialize(this->FrustumFar, local_Ar);
    }
    {
        FArchive local_Ar(content["Resolution"], Ar);
        TStaticSerializer<decltype(this->Resolution)>::Serialize(this->Resolution, local_Ar);
    }
    {
        FArchive local_Ar(content["WorldToClip"], Ar);
        TStaticSerializer<decltype(this->WorldToClip)>::Serialize(this->WorldToClip, local_Ar);
    }
}

void nilou::FShadowMappingParameters::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("FrustumFar"))
    {
        FArchive local_Ar(content["FrustumFar"], Ar);
        TStaticSerializer<decltype(this->FrustumFar)>::Deserialize(this->FrustumFar, local_Ar);
    }
    if (content.contains("Resolution"))
    {
        FArchive local_Ar(content["Resolution"], Ar);
        TStaticSerializer<decltype(this->Resolution)>::Deserialize(this->Resolution, local_Ar);
    }
    if (content.contains("WorldToClip"))
    {
        FArchive local_Ar(content["WorldToClip"], Ar);
        TStaticSerializer<decltype(this->WorldToClip)>::Deserialize(this->WorldToClip, local_Ar);
    }
    
}
