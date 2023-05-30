#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FDirectionalShadowMappingBlock::StaticClass_ = nullptr;
const NClass *nilou::FDirectionalShadowMappingBlock::GetClass() const 
{ 
    return nilou::FDirectionalShadowMappingBlock::StaticClass(); 
}
const NClass *nilou::FDirectionalShadowMappingBlock::StaticClass()
{
    return nilou::FDirectionalShadowMappingBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FDirectionalShadowMappingBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FDirectionalShadowMappingBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FDirectionalShadowMappingBlock>();
		Mngr.AddField<&nilou::FDirectionalShadowMappingBlock::Frustums>("Frustums");
		Mngr.AddMethod<&nilou::FDirectionalShadowMappingBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FDirectionalShadowMappingBlock::Serialize>("Serialize");
;
        nilou::FDirectionalShadowMappingBlock::StaticClass_->Type = Type_of<nilou::FDirectionalShadowMappingBlock>;
        nilou::FDirectionalShadowMappingBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FDirectionalShadowMappingBlock>);
    }

    static TClassRegistry<nilou::FDirectionalShadowMappingBlock> Dummy;
};
TClassRegistry<nilou::FDirectionalShadowMappingBlock> Dummy = TClassRegistry<nilou::FDirectionalShadowMappingBlock>("nilou::FDirectionalShadowMappingBlock");



void nilou::FDirectionalShadowMappingBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Frustums"], Ar);
        TStaticSerializer<decltype(this->Frustums)>::Serialize(this->Frustums, local_Ar);
    }
}

void nilou::FDirectionalShadowMappingBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Frustums"))
    {
        FArchive local_Ar(content["Frustums"], Ar);
        TStaticSerializer<decltype(this->Frustums)>::Deserialize(this->Frustums, local_Ar);
    }
    
}
