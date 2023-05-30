#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::StaticClass_ = nullptr;
const NClass *nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::GetClass() const 
{ 
    return nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::StaticClass(); 
}
const NClass *nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::StaticClass()
{
    return nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock>();
		Mngr.AddField<&nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::TextureSize>("TextureSize");
		Mngr.AddField<&nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::roughness>("roughness");
		Mngr.AddMethod<&nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::Serialize>("Serialize");
;
        nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::StaticClass_->Type = Type_of<nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock>;
        nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock>);
    }

    static TClassRegistry<nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock> Dummy;
};
TClassRegistry<nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock> Dummy = TClassRegistry<nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock>("nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock");



void nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["TextureSize"], Ar);
        TStaticSerializer<decltype(this->TextureSize)>::Serialize(this->TextureSize, local_Ar);
    }
    {
        FArchive local_Ar(content["roughness"], Ar);
        TStaticSerializer<decltype(this->roughness)>::Serialize(this->roughness, local_Ar);
    }
}

void nilou::UReflectionProbeComponent::PrefilteredEnvTextureShaderBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("TextureSize"))
    {
        FArchive local_Ar(content["TextureSize"], Ar);
        TStaticSerializer<decltype(this->TextureSize)>::Deserialize(this->TextureSize, local_Ar);
    }
    if (content.contains("roughness"))
    {
        FArchive local_Ar(content["roughness"], Ar);
        TStaticSerializer<decltype(this->roughness)>::Deserialize(this->roughness, local_Ar);
    }
    
}
