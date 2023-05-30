#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FOceanFastFourierTransformParameters::StaticClass_ = nullptr;
const NClass *nilou::FOceanFastFourierTransformParameters::GetClass() const 
{ 
    return nilou::FOceanFastFourierTransformParameters::StaticClass(); 
}
const NClass *nilou::FOceanFastFourierTransformParameters::StaticClass()
{
    return nilou::FOceanFastFourierTransformParameters::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FOceanFastFourierTransformParameters>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FOceanFastFourierTransformParameters::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FOceanFastFourierTransformParameters>();
		Mngr.AddField<&nilou::FOceanFastFourierTransformParameters::Amplitude>("Amplitude");
		Mngr.AddField<&nilou::FOceanFastFourierTransformParameters::DisplacementTextureSize>("DisplacementTextureSize");
		Mngr.AddField<&nilou::FOceanFastFourierTransformParameters::N>("N");
		Mngr.AddField<&nilou::FOceanFastFourierTransformParameters::Time>("Time");
		Mngr.AddField<&nilou::FOceanFastFourierTransformParameters::WindDirection>("WindDirection");
		Mngr.AddField<&nilou::FOceanFastFourierTransformParameters::WindSpeed>("WindSpeed");
		Mngr.AddMethod<&nilou::FOceanFastFourierTransformParameters::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FOceanFastFourierTransformParameters::Serialize>("Serialize");
;
        nilou::FOceanFastFourierTransformParameters::StaticClass_->Type = Type_of<nilou::FOceanFastFourierTransformParameters>;
        nilou::FOceanFastFourierTransformParameters::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FOceanFastFourierTransformParameters>);
    }

    static TClassRegistry<nilou::FOceanFastFourierTransformParameters> Dummy;
};
TClassRegistry<nilou::FOceanFastFourierTransformParameters> Dummy = TClassRegistry<nilou::FOceanFastFourierTransformParameters>("nilou::FOceanFastFourierTransformParameters");



void nilou::FOceanFastFourierTransformParameters::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Amplitude"], Ar);
        TStaticSerializer<decltype(this->Amplitude)>::Serialize(this->Amplitude, local_Ar);
    }
    {
        FArchive local_Ar(content["DisplacementTextureSize"], Ar);
        TStaticSerializer<decltype(this->DisplacementTextureSize)>::Serialize(this->DisplacementTextureSize, local_Ar);
    }
    {
        FArchive local_Ar(content["N"], Ar);
        TStaticSerializer<decltype(this->N)>::Serialize(this->N, local_Ar);
    }
    {
        FArchive local_Ar(content["Time"], Ar);
        TStaticSerializer<decltype(this->Time)>::Serialize(this->Time, local_Ar);
    }
    {
        FArchive local_Ar(content["WindDirection"], Ar);
        TStaticSerializer<decltype(this->WindDirection)>::Serialize(this->WindDirection, local_Ar);
    }
    {
        FArchive local_Ar(content["WindSpeed"], Ar);
        TStaticSerializer<decltype(this->WindSpeed)>::Serialize(this->WindSpeed, local_Ar);
    }
}

void nilou::FOceanFastFourierTransformParameters::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Amplitude"))
    {
        FArchive local_Ar(content["Amplitude"], Ar);
        TStaticSerializer<decltype(this->Amplitude)>::Deserialize(this->Amplitude, local_Ar);
    }
    if (content.contains("DisplacementTextureSize"))
    {
        FArchive local_Ar(content["DisplacementTextureSize"], Ar);
        TStaticSerializer<decltype(this->DisplacementTextureSize)>::Deserialize(this->DisplacementTextureSize, local_Ar);
    }
    if (content.contains("N"))
    {
        FArchive local_Ar(content["N"], Ar);
        TStaticSerializer<decltype(this->N)>::Deserialize(this->N, local_Ar);
    }
    if (content.contains("Time"))
    {
        FArchive local_Ar(content["Time"], Ar);
        TStaticSerializer<decltype(this->Time)>::Deserialize(this->Time, local_Ar);
    }
    if (content.contains("WindDirection"))
    {
        FArchive local_Ar(content["WindDirection"], Ar);
        TStaticSerializer<decltype(this->WindDirection)>::Deserialize(this->WindDirection, local_Ar);
    }
    if (content.contains("WindSpeed"))
    {
        FArchive local_Ar(content["WindSpeed"], Ar);
        TStaticSerializer<decltype(this->WindSpeed)>::Deserialize(this->WindSpeed, local_Ar);
    }
    
}
