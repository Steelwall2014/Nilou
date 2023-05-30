#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FPointShadowMappingBlock::StaticClass_ = nullptr;
const NClass *nilou::FPointShadowMappingBlock::GetClass() const 
{ 
    return nilou::FPointShadowMappingBlock::StaticClass(); 
}
const NClass *nilou::FPointShadowMappingBlock::StaticClass()
{
    return nilou::FPointShadowMappingBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FPointShadowMappingBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FPointShadowMappingBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FPointShadowMappingBlock>();
		Mngr.AddField<&nilou::FPointShadowMappingBlock::Frustums>("Frustums");
		Mngr.AddMethod<&nilou::FPointShadowMappingBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FPointShadowMappingBlock::Serialize>("Serialize");
;
        nilou::FPointShadowMappingBlock::StaticClass_->Type = Type_of<nilou::FPointShadowMappingBlock>;
        nilou::FPointShadowMappingBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FPointShadowMappingBlock>);
    }

    static TClassRegistry<nilou::FPointShadowMappingBlock> Dummy;
};
TClassRegistry<nilou::FPointShadowMappingBlock> Dummy = TClassRegistry<nilou::FPointShadowMappingBlock>("nilou::FPointShadowMappingBlock");



void nilou::FPointShadowMappingBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Frustums"], Ar);
        TStaticSerializer<decltype(this->Frustums)>::Serialize(this->Frustums, local_Ar);
    }
}

void nilou::FPointShadowMappingBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Frustums"))
    {
        FArchive local_Ar(content["Frustums"], Ar);
        TStaticSerializer<decltype(this->Frustums)>::Deserialize(this->Frustums, local_Ar);
    }
    
}
