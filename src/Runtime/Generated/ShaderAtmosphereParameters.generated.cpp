#include "D:/Nilou/src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ShaderAtmosphereParameters::StaticClass_ = nullptr;
const NClass *nilou::ShaderAtmosphereParameters::GetClass() const 
{ 
    return nilou::ShaderAtmosphereParameters::StaticClass(); 
}
const NClass *nilou::ShaderAtmosphereParameters::StaticClass()
{
    return nilou::ShaderAtmosphereParameters::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ShaderAtmosphereParameters>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ShaderAtmosphereParameters::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ShaderAtmosphereParameters>();
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::AbsorptionDensity>("AbsorptionDensity");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::AbsorptionExtinction>("AbsorptionExtinction");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::BottomRadius>("BottomRadius");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::GroundAlbedo>("GroundAlbedo");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::MieDensity>("MieDensity");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::MieExtinction>("MieExtinction");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::MiePhaseFunction_g>("MiePhaseFunction_g");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::MieScattering>("MieScattering");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::Mu_s_Min>("Mu_s_Min");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::RayleighDensity>("RayleighDensity");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::RayleighScattering>("RayleighScattering");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::SolarIrradiance>("SolarIrradiance");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::SunAngularRadius>("SunAngularRadius");
		Mngr.AddField<&nilou::ShaderAtmosphereParameters::TopRadius>("TopRadius");
		Mngr.AddMethod<&nilou::ShaderAtmosphereParameters::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::ShaderAtmosphereParameters::Serialize>("Serialize");
;
        nilou::ShaderAtmosphereParameters::StaticClass_->Type = Type_of<nilou::ShaderAtmosphereParameters>;
        nilou::ShaderAtmosphereParameters::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ShaderAtmosphereParameters>);
    }

    static TClassRegistry<nilou::ShaderAtmosphereParameters> Dummy;
};
TClassRegistry<nilou::ShaderAtmosphereParameters> Dummy = TClassRegistry<nilou::ShaderAtmosphereParameters>("nilou::ShaderAtmosphereParameters");



void nilou::ShaderAtmosphereParameters::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["AbsorptionDensity"], Ar);
        this->AbsorptionDensity.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["AbsorptionExtinction"], Ar);
        TStaticSerializer<decltype(this->AbsorptionExtinction)>::Serialize(this->AbsorptionExtinction, local_Ar);
    }
    {
        FArchive local_Ar(content["BottomRadius"], Ar);
        TStaticSerializer<decltype(this->BottomRadius)>::Serialize(this->BottomRadius, local_Ar);
    }
    {
        FArchive local_Ar(content["GroundAlbedo"], Ar);
        TStaticSerializer<decltype(this->GroundAlbedo)>::Serialize(this->GroundAlbedo, local_Ar);
    }
    {
        FArchive local_Ar(content["MieDensity"], Ar);
        this->MieDensity.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["MieExtinction"], Ar);
        TStaticSerializer<decltype(this->MieExtinction)>::Serialize(this->MieExtinction, local_Ar);
    }
    {
        FArchive local_Ar(content["MiePhaseFunction_g"], Ar);
        TStaticSerializer<decltype(this->MiePhaseFunction_g)>::Serialize(this->MiePhaseFunction_g, local_Ar);
    }
    {
        FArchive local_Ar(content["MieScattering"], Ar);
        TStaticSerializer<decltype(this->MieScattering)>::Serialize(this->MieScattering, local_Ar);
    }
    {
        FArchive local_Ar(content["Mu_s_Min"], Ar);
        TStaticSerializer<decltype(this->Mu_s_Min)>::Serialize(this->Mu_s_Min, local_Ar);
    }
    {
        FArchive local_Ar(content["RayleighDensity"], Ar);
        this->RayleighDensity.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["RayleighScattering"], Ar);
        TStaticSerializer<decltype(this->RayleighScattering)>::Serialize(this->RayleighScattering, local_Ar);
    }
    {
        FArchive local_Ar(content["SolarIrradiance"], Ar);
        TStaticSerializer<decltype(this->SolarIrradiance)>::Serialize(this->SolarIrradiance, local_Ar);
    }
    {
        FArchive local_Ar(content["SunAngularRadius"], Ar);
        TStaticSerializer<decltype(this->SunAngularRadius)>::Serialize(this->SunAngularRadius, local_Ar);
    }
    {
        FArchive local_Ar(content["TopRadius"], Ar);
        TStaticSerializer<decltype(this->TopRadius)>::Serialize(this->TopRadius, local_Ar);
    }
}

void nilou::ShaderAtmosphereParameters::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("AbsorptionDensity"))
    {
        FArchive local_Ar(content["AbsorptionDensity"], Ar);
        this->AbsorptionDensity.Deserialize(local_Ar);
    }
    if (content.contains("AbsorptionExtinction"))
    {
        FArchive local_Ar(content["AbsorptionExtinction"], Ar);
        TStaticSerializer<decltype(this->AbsorptionExtinction)>::Deserialize(this->AbsorptionExtinction, local_Ar);
    }
    if (content.contains("BottomRadius"))
    {
        FArchive local_Ar(content["BottomRadius"], Ar);
        TStaticSerializer<decltype(this->BottomRadius)>::Deserialize(this->BottomRadius, local_Ar);
    }
    if (content.contains("GroundAlbedo"))
    {
        FArchive local_Ar(content["GroundAlbedo"], Ar);
        TStaticSerializer<decltype(this->GroundAlbedo)>::Deserialize(this->GroundAlbedo, local_Ar);
    }
    if (content.contains("MieDensity"))
    {
        FArchive local_Ar(content["MieDensity"], Ar);
        this->MieDensity.Deserialize(local_Ar);
    }
    if (content.contains("MieExtinction"))
    {
        FArchive local_Ar(content["MieExtinction"], Ar);
        TStaticSerializer<decltype(this->MieExtinction)>::Deserialize(this->MieExtinction, local_Ar);
    }
    if (content.contains("MiePhaseFunction_g"))
    {
        FArchive local_Ar(content["MiePhaseFunction_g"], Ar);
        TStaticSerializer<decltype(this->MiePhaseFunction_g)>::Deserialize(this->MiePhaseFunction_g, local_Ar);
    }
    if (content.contains("MieScattering"))
    {
        FArchive local_Ar(content["MieScattering"], Ar);
        TStaticSerializer<decltype(this->MieScattering)>::Deserialize(this->MieScattering, local_Ar);
    }
    if (content.contains("Mu_s_Min"))
    {
        FArchive local_Ar(content["Mu_s_Min"], Ar);
        TStaticSerializer<decltype(this->Mu_s_Min)>::Deserialize(this->Mu_s_Min, local_Ar);
    }
    if (content.contains("RayleighDensity"))
    {
        FArchive local_Ar(content["RayleighDensity"], Ar);
        this->RayleighDensity.Deserialize(local_Ar);
    }
    if (content.contains("RayleighScattering"))
    {
        FArchive local_Ar(content["RayleighScattering"], Ar);
        TStaticSerializer<decltype(this->RayleighScattering)>::Deserialize(this->RayleighScattering, local_Ar);
    }
    if (content.contains("SolarIrradiance"))
    {
        FArchive local_Ar(content["SolarIrradiance"], Ar);
        TStaticSerializer<decltype(this->SolarIrradiance)>::Deserialize(this->SolarIrradiance, local_Ar);
    }
    if (content.contains("SunAngularRadius"))
    {
        FArchive local_Ar(content["SunAngularRadius"], Ar);
        TStaticSerializer<decltype(this->SunAngularRadius)>::Deserialize(this->SunAngularRadius, local_Ar);
    }
    if (content.contains("TopRadius"))
    {
        FArchive local_Ar(content["TopRadius"], Ar);
        TStaticSerializer<decltype(this->TopRadius)>::Deserialize(this->TopRadius, local_Ar);
    }
    
}
