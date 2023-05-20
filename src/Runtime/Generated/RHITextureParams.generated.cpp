#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::RHITextureParams::StaticClass_ = nullptr;
const NClass *nilou::RHITextureParams::GetClass() const 
{ 
    return nilou::RHITextureParams::StaticClass(); 
}
const NClass *nilou::RHITextureParams::StaticClass()
{
    return nilou::RHITextureParams::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::RHITextureParams>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::RHITextureParams::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::RHITextureParams>();
		Mngr.AddField<&nilou::RHITextureParams::Mag_Filter>("Mag_Filter");
		Mngr.AddField<&nilou::RHITextureParams::Min_Filter>("Min_Filter");
		Mngr.AddField<&nilou::RHITextureParams::Wrap_R>("Wrap_R");
		Mngr.AddField<&nilou::RHITextureParams::Wrap_S>("Wrap_S");
		Mngr.AddField<&nilou::RHITextureParams::Wrap_T>("Wrap_T");
;
        nilou::RHITextureParams::StaticClass_->Type = Type_of<nilou::RHITextureParams>;
        nilou::RHITextureParams::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::RHITextureParams>);
    }

    static TClassRegistry<nilou::RHITextureParams> Dummy;
};
TClassRegistry<nilou::RHITextureParams> Dummy = TClassRegistry<nilou::RHITextureParams>("nilou::RHITextureParams");



void nilou::RHITextureParams::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Mag_Filter"], Ar);
        TStaticSerializer<decltype(this->Mag_Filter)>::Serialize(this->Mag_Filter, local_Ar);
    }
    {
        FArchive local_Ar(content["Min_Filter"], Ar);
        TStaticSerializer<decltype(this->Min_Filter)>::Serialize(this->Min_Filter, local_Ar);
    }
    {
        FArchive local_Ar(content["Wrap_R"], Ar);
        TStaticSerializer<decltype(this->Wrap_R)>::Serialize(this->Wrap_R, local_Ar);
    }
    {
        FArchive local_Ar(content["Wrap_S"], Ar);
        TStaticSerializer<decltype(this->Wrap_S)>::Serialize(this->Wrap_S, local_Ar);
    }
    {
        FArchive local_Ar(content["Wrap_T"], Ar);
        TStaticSerializer<decltype(this->Wrap_T)>::Serialize(this->Wrap_T, local_Ar);
    }
}

void nilou::RHITextureParams::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Mag_Filter"))
    {
        FArchive local_Ar(content["Mag_Filter"], Ar);
        TStaticSerializer<decltype(this->Mag_Filter)>::Deserialize(this->Mag_Filter, local_Ar);
    }
    if (content.contains("Min_Filter"))
    {
        FArchive local_Ar(content["Min_Filter"], Ar);
        TStaticSerializer<decltype(this->Min_Filter)>::Deserialize(this->Min_Filter, local_Ar);
    }
    if (content.contains("Wrap_R"))
    {
        FArchive local_Ar(content["Wrap_R"], Ar);
        TStaticSerializer<decltype(this->Wrap_R)>::Deserialize(this->Wrap_R, local_Ar);
    }
    if (content.contains("Wrap_S"))
    {
        FArchive local_Ar(content["Wrap_S"], Ar);
        TStaticSerializer<decltype(this->Wrap_S)>::Deserialize(this->Wrap_S, local_Ar);
    }
    if (content.contains("Wrap_T"))
    {
        FArchive local_Ar(content["Wrap_T"], Ar);
        TStaticSerializer<decltype(this->Wrap_T)>::Deserialize(this->Wrap_T, local_Ar);
    }
    
}
