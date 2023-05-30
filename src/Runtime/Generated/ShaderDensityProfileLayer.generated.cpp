#include "D:/Nilou/src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ShaderDensityProfileLayer::StaticClass_ = nullptr;
const NClass *nilou::ShaderDensityProfileLayer::GetClass() const 
{ 
    return nilou::ShaderDensityProfileLayer::StaticClass(); 
}
const NClass *nilou::ShaderDensityProfileLayer::StaticClass()
{
    return nilou::ShaderDensityProfileLayer::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ShaderDensityProfileLayer>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ShaderDensityProfileLayer::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ShaderDensityProfileLayer>();
		Mngr.AddField<&nilou::ShaderDensityProfileLayer::constant_term>("constant_term");
		Mngr.AddField<&nilou::ShaderDensityProfileLayer::exp_scale>("exp_scale");
		Mngr.AddField<&nilou::ShaderDensityProfileLayer::exp_term>("exp_term");
		Mngr.AddField<&nilou::ShaderDensityProfileLayer::linear_term>("linear_term");
		Mngr.AddField<&nilou::ShaderDensityProfileLayer::width>("width");
		Mngr.AddMethod<&nilou::ShaderDensityProfileLayer::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::ShaderDensityProfileLayer::Serialize>("Serialize");
;
        nilou::ShaderDensityProfileLayer::StaticClass_->Type = Type_of<nilou::ShaderDensityProfileLayer>;
        nilou::ShaderDensityProfileLayer::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ShaderDensityProfileLayer>);
    }

    static TClassRegistry<nilou::ShaderDensityProfileLayer> Dummy;
};
TClassRegistry<nilou::ShaderDensityProfileLayer> Dummy = TClassRegistry<nilou::ShaderDensityProfileLayer>("nilou::ShaderDensityProfileLayer");



void nilou::ShaderDensityProfileLayer::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["constant_term"], Ar);
        TStaticSerializer<decltype(this->constant_term)>::Serialize(this->constant_term, local_Ar);
    }
    {
        FArchive local_Ar(content["exp_scale"], Ar);
        TStaticSerializer<decltype(this->exp_scale)>::Serialize(this->exp_scale, local_Ar);
    }
    {
        FArchive local_Ar(content["exp_term"], Ar);
        TStaticSerializer<decltype(this->exp_term)>::Serialize(this->exp_term, local_Ar);
    }
    {
        FArchive local_Ar(content["linear_term"], Ar);
        TStaticSerializer<decltype(this->linear_term)>::Serialize(this->linear_term, local_Ar);
    }
    {
        FArchive local_Ar(content["width"], Ar);
        TStaticSerializer<decltype(this->width)>::Serialize(this->width, local_Ar);
    }
}

void nilou::ShaderDensityProfileLayer::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("constant_term"))
    {
        FArchive local_Ar(content["constant_term"], Ar);
        TStaticSerializer<decltype(this->constant_term)>::Deserialize(this->constant_term, local_Ar);
    }
    if (content.contains("exp_scale"))
    {
        FArchive local_Ar(content["exp_scale"], Ar);
        TStaticSerializer<decltype(this->exp_scale)>::Deserialize(this->exp_scale, local_Ar);
    }
    if (content.contains("exp_term"))
    {
        FArchive local_Ar(content["exp_term"], Ar);
        TStaticSerializer<decltype(this->exp_term)>::Deserialize(this->exp_term, local_Ar);
    }
    if (content.contains("linear_term"))
    {
        FArchive local_Ar(content["linear_term"], Ar);
        TStaticSerializer<decltype(this->linear_term)>::Deserialize(this->linear_term, local_Ar);
    }
    if (content.contains("width"))
    {
        FArchive local_Ar(content["width"], Ar);
        TStaticSerializer<decltype(this->width)>::Deserialize(this->width, local_Ar);
    }
    
}
