#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FLightAttenParameters::StaticClass_ = nullptr;
const NClass *nilou::FLightAttenParameters::GetClass() const 
{ 
    return nilou::FLightAttenParameters::StaticClass(); 
}
const NClass *nilou::FLightAttenParameters::StaticClass()
{
    return nilou::FLightAttenParameters::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FLightAttenParameters>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FLightAttenParameters::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FLightAttenParameters>();
		Mngr.AddField<&nilou::FLightAttenParameters::AttenCurveParams>("AttenCurveParams");
		Mngr.AddField<&nilou::FLightAttenParameters::AttenCurveScale>("AttenCurveScale");
		Mngr.AddField<&nilou::FLightAttenParameters::AttenCurveType>("AttenCurveType");
		Mngr.AddMethod<&nilou::FLightAttenParameters::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FLightAttenParameters::Serialize>("Serialize");
;
        nilou::FLightAttenParameters::StaticClass_->Type = Type_of<nilou::FLightAttenParameters>;
        nilou::FLightAttenParameters::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FLightAttenParameters>);
    }

    static TClassRegistry<nilou::FLightAttenParameters> Dummy;
};
TClassRegistry<nilou::FLightAttenParameters> Dummy = TClassRegistry<nilou::FLightAttenParameters>("nilou::FLightAttenParameters");



void nilou::FLightAttenParameters::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["AttenCurveParams"], Ar);
        TStaticSerializer<decltype(this->AttenCurveParams)>::Serialize(this->AttenCurveParams, local_Ar);
    }
    {
        FArchive local_Ar(content["AttenCurveScale"], Ar);
        TStaticSerializer<decltype(this->AttenCurveScale)>::Serialize(this->AttenCurveScale, local_Ar);
    }
    {
        FArchive local_Ar(content["AttenCurveType"], Ar);
        TStaticSerializer<decltype(this->AttenCurveType)>::Serialize(this->AttenCurveType, local_Ar);
    }
}

void nilou::FLightAttenParameters::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("AttenCurveParams"))
    {
        FArchive local_Ar(content["AttenCurveParams"], Ar);
        TStaticSerializer<decltype(this->AttenCurveParams)>::Deserialize(this->AttenCurveParams, local_Ar);
    }
    if (content.contains("AttenCurveScale"))
    {
        FArchive local_Ar(content["AttenCurveScale"], Ar);
        TStaticSerializer<decltype(this->AttenCurveScale)>::Deserialize(this->AttenCurveScale, local_Ar);
    }
    if (content.contains("AttenCurveType"))
    {
        FArchive local_Ar(content["AttenCurveType"], Ar);
        TStaticSerializer<decltype(this->AttenCurveType)>::Deserialize(this->AttenCurveType, local_Ar);
    }
    
}
