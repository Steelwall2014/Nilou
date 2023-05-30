#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FSpotShadowMappingBlock::StaticClass_ = nullptr;
const NClass *nilou::FSpotShadowMappingBlock::GetClass() const 
{ 
    return nilou::FSpotShadowMappingBlock::StaticClass(); 
}
const NClass *nilou::FSpotShadowMappingBlock::StaticClass()
{
    return nilou::FSpotShadowMappingBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FSpotShadowMappingBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FSpotShadowMappingBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FSpotShadowMappingBlock>();
		Mngr.AddField<&nilou::FSpotShadowMappingBlock::Frustums>("Frustums");
		Mngr.AddMethod<&nilou::FSpotShadowMappingBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FSpotShadowMappingBlock::Serialize>("Serialize");
;
        nilou::FSpotShadowMappingBlock::StaticClass_->Type = Type_of<nilou::FSpotShadowMappingBlock>;
        nilou::FSpotShadowMappingBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FSpotShadowMappingBlock>);
    }

    static TClassRegistry<nilou::FSpotShadowMappingBlock> Dummy;
};
TClassRegistry<nilou::FSpotShadowMappingBlock> Dummy = TClassRegistry<nilou::FSpotShadowMappingBlock>("nilou::FSpotShadowMappingBlock");



void nilou::FSpotShadowMappingBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Frustums"], Ar);
        TStaticSerializer<decltype(this->Frustums)>::Serialize(this->Frustums, local_Ar);
    }
}

void nilou::FSpotShadowMappingBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Frustums"))
    {
        FArchive local_Ar(content["Frustums"], Ar);
        TStaticSerializer<decltype(this->Frustums)>::Deserialize(this->Frustums, local_Ar);
    }
    
}
