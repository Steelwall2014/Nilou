#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::StaticClass_ = nullptr;
const NClass *nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::GetClass() const 
{ 
    return nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::StaticClass(); 
}
const NClass *nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::StaticClass()
{
    return nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock>();
		Mngr.AddField<&nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::TextureSize>("TextureSize");
		Mngr.AddMethod<&nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::Serialize>("Serialize");
;
        nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::StaticClass_->Type = Type_of<nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock>;
        nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock>);
    }

    static TClassRegistry<nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock> Dummy;
};
TClassRegistry<nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock> Dummy = TClassRegistry<nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock>("nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock");



void nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["TextureSize"], Ar);
        TStaticSerializer<decltype(this->TextureSize)>::Serialize(this->TextureSize, local_Ar);
    }
}

void nilou::UReflectionProbeComponent::IrradianceEnvTextureShaderBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("TextureSize"))
    {
        FArchive local_Ar(content["TextureSize"], Ar);
        TStaticSerializer<decltype(this->TextureSize)>::Deserialize(this->TextureSize, local_Ar);
    }
    
}
