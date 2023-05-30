#include "D:/Nilou/src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ShaderDensityProfile::StaticClass_ = nullptr;
const NClass *nilou::ShaderDensityProfile::GetClass() const 
{ 
    return nilou::ShaderDensityProfile::StaticClass(); 
}
const NClass *nilou::ShaderDensityProfile::StaticClass()
{
    return nilou::ShaderDensityProfile::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ShaderDensityProfile>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ShaderDensityProfile::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ShaderDensityProfile>();
		Mngr.AddField<&nilou::ShaderDensityProfile::layers>("layers");
		Mngr.AddMethod<&nilou::ShaderDensityProfile::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::ShaderDensityProfile::Serialize>("Serialize");
;
        nilou::ShaderDensityProfile::StaticClass_->Type = Type_of<nilou::ShaderDensityProfile>;
        nilou::ShaderDensityProfile::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ShaderDensityProfile>);
    }

    static TClassRegistry<nilou::ShaderDensityProfile> Dummy;
};
TClassRegistry<nilou::ShaderDensityProfile> Dummy = TClassRegistry<nilou::ShaderDensityProfile>("nilou::ShaderDensityProfile");



void nilou::ShaderDensityProfile::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["layers"], Ar);
        TStaticSerializer<decltype(this->layers)>::Serialize(this->layers, local_Ar);
    }
}

void nilou::ShaderDensityProfile::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("layers"))
    {
        FArchive local_Ar(content["layers"], Ar);
        TStaticSerializer<decltype(this->layers)>::Deserialize(this->layers, local_Ar);
    }
    
}
