#include "D:/Nilou/src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ShaderAtmosphereParametersBlock::StaticClass_ = nullptr;
const NClass *nilou::ShaderAtmosphereParametersBlock::GetClass() const 
{ 
    return nilou::ShaderAtmosphereParametersBlock::StaticClass(); 
}
const NClass *nilou::ShaderAtmosphereParametersBlock::StaticClass()
{
    return nilou::ShaderAtmosphereParametersBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ShaderAtmosphereParametersBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ShaderAtmosphereParametersBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ShaderAtmosphereParametersBlock>();
		Mngr.AddField<&nilou::ShaderAtmosphereParametersBlock::ATMOSPHERE>("ATMOSPHERE");
		Mngr.AddMethod<&nilou::ShaderAtmosphereParametersBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::ShaderAtmosphereParametersBlock::Serialize>("Serialize");
;
        nilou::ShaderAtmosphereParametersBlock::StaticClass_->Type = Type_of<nilou::ShaderAtmosphereParametersBlock>;
        nilou::ShaderAtmosphereParametersBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ShaderAtmosphereParametersBlock>);
    }

    static TClassRegistry<nilou::ShaderAtmosphereParametersBlock> Dummy;
};
TClassRegistry<nilou::ShaderAtmosphereParametersBlock> Dummy = TClassRegistry<nilou::ShaderAtmosphereParametersBlock>("nilou::ShaderAtmosphereParametersBlock");



void nilou::ShaderAtmosphereParametersBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["ATMOSPHERE"], Ar);
        this->ATMOSPHERE.Serialize(local_Ar);
    }
}

void nilou::ShaderAtmosphereParametersBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("ATMOSPHERE"))
    {
        FArchive local_Ar(content["ATMOSPHERE"], Ar);
        this->ATMOSPHERE.Deserialize(local_Ar);
    }
    
}
