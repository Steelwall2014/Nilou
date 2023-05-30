#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FLightShaderParameters::StaticClass_ = nullptr;
const NClass *nilou::FLightShaderParameters::GetClass() const 
{ 
    return nilou::FLightShaderParameters::StaticClass(); 
}
const NClass *nilou::FLightShaderParameters::StaticClass()
{
    return nilou::FLightShaderParameters::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FLightShaderParameters>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FLightShaderParameters::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FLightShaderParameters>();
		Mngr.AddField<&nilou::FLightShaderParameters::lightAngleAttenParams>("lightAngleAttenParams");
		Mngr.AddField<&nilou::FLightShaderParameters::lightCastShadow>("lightCastShadow");
		Mngr.AddField<&nilou::FLightShaderParameters::lightDirection>("lightDirection");
		Mngr.AddField<&nilou::FLightShaderParameters::lightDistAttenParams>("lightDistAttenParams");
		Mngr.AddField<&nilou::FLightShaderParameters::lightIntensity>("lightIntensity");
		Mngr.AddField<&nilou::FLightShaderParameters::lightPosition>("lightPosition");
		Mngr.AddField<&nilou::FLightShaderParameters::lightType>("lightType");
		Mngr.AddMethod<&nilou::FLightShaderParameters::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FLightShaderParameters::Serialize>("Serialize");
;
        nilou::FLightShaderParameters::StaticClass_->Type = Type_of<nilou::FLightShaderParameters>;
        nilou::FLightShaderParameters::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FLightShaderParameters>);
    }

    static TClassRegistry<nilou::FLightShaderParameters> Dummy;
};
TClassRegistry<nilou::FLightShaderParameters> Dummy = TClassRegistry<nilou::FLightShaderParameters>("nilou::FLightShaderParameters");



void nilou::FLightShaderParameters::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["lightAngleAttenParams"], Ar);
        this->lightAngleAttenParams.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["lightCastShadow"], Ar);
        TStaticSerializer<decltype(this->lightCastShadow)>::Serialize(this->lightCastShadow, local_Ar);
    }
    {
        FArchive local_Ar(content["lightDirection"], Ar);
        TStaticSerializer<decltype(this->lightDirection)>::Serialize(this->lightDirection, local_Ar);
    }
    {
        FArchive local_Ar(content["lightDistAttenParams"], Ar);
        this->lightDistAttenParams.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["lightIntensity"], Ar);
        TStaticSerializer<decltype(this->lightIntensity)>::Serialize(this->lightIntensity, local_Ar);
    }
    {
        FArchive local_Ar(content["lightPosition"], Ar);
        TStaticSerializer<decltype(this->lightPosition)>::Serialize(this->lightPosition, local_Ar);
    }
    {
        FArchive local_Ar(content["lightType"], Ar);
        TStaticSerializer<decltype(this->lightType)>::Serialize(this->lightType, local_Ar);
    }
}

void nilou::FLightShaderParameters::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("lightAngleAttenParams"))
    {
        FArchive local_Ar(content["lightAngleAttenParams"], Ar);
        this->lightAngleAttenParams.Deserialize(local_Ar);
    }
    if (content.contains("lightCastShadow"))
    {
        FArchive local_Ar(content["lightCastShadow"], Ar);
        TStaticSerializer<decltype(this->lightCastShadow)>::Deserialize(this->lightCastShadow, local_Ar);
    }
    if (content.contains("lightDirection"))
    {
        FArchive local_Ar(content["lightDirection"], Ar);
        TStaticSerializer<decltype(this->lightDirection)>::Deserialize(this->lightDirection, local_Ar);
    }
    if (content.contains("lightDistAttenParams"))
    {
        FArchive local_Ar(content["lightDistAttenParams"], Ar);
        this->lightDistAttenParams.Deserialize(local_Ar);
    }
    if (content.contains("lightIntensity"))
    {
        FArchive local_Ar(content["lightIntensity"], Ar);
        TStaticSerializer<decltype(this->lightIntensity)>::Deserialize(this->lightIntensity, local_Ar);
    }
    if (content.contains("lightPosition"))
    {
        FArchive local_Ar(content["lightPosition"], Ar);
        TStaticSerializer<decltype(this->lightPosition)>::Deserialize(this->lightPosition, local_Ar);
    }
    if (content.contains("lightType"))
    {
        FArchive local_Ar(content["lightType"], Ar);
        TStaticSerializer<decltype(this->lightType)>::Deserialize(this->lightType, local_Ar);
    }
    
}
