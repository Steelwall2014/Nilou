#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FCreateNodeListBlock::StaticClass_ = nullptr;
const NClass *nilou::FCreateNodeListBlock::GetClass() const 
{ 
    return nilou::FCreateNodeListBlock::StaticClass(); 
}
const NClass *nilou::FCreateNodeListBlock::StaticClass()
{
    return nilou::FCreateNodeListBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FCreateNodeListBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FCreateNodeListBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FCreateNodeListBlock>();
		Mngr.AddField<&nilou::FCreateNodeListBlock::MaxLOD>("MaxLOD");
		Mngr.AddField<&nilou::FCreateNodeListBlock::PassLOD>("PassLOD");
		Mngr.AddField<&nilou::FCreateNodeListBlock::ScreenSizeDenominator>("ScreenSizeDenominator");
		Mngr.AddMethod<&nilou::FCreateNodeListBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FCreateNodeListBlock::Serialize>("Serialize");
;
        nilou::FCreateNodeListBlock::StaticClass_->Type = Type_of<nilou::FCreateNodeListBlock>;
        nilou::FCreateNodeListBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FCreateNodeListBlock>);
    }

    static TClassRegistry<nilou::FCreateNodeListBlock> Dummy;
};
TClassRegistry<nilou::FCreateNodeListBlock> Dummy = TClassRegistry<nilou::FCreateNodeListBlock>("nilou::FCreateNodeListBlock");



void nilou::FCreateNodeListBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["MaxLOD"], Ar);
        TStaticSerializer<decltype(this->MaxLOD)>::Serialize(this->MaxLOD, local_Ar);
    }
    {
        FArchive local_Ar(content["PassLOD"], Ar);
        TStaticSerializer<decltype(this->PassLOD)>::Serialize(this->PassLOD, local_Ar);
    }
    {
        FArchive local_Ar(content["ScreenSizeDenominator"], Ar);
        TStaticSerializer<decltype(this->ScreenSizeDenominator)>::Serialize(this->ScreenSizeDenominator, local_Ar);
    }
}

void nilou::FCreateNodeListBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("MaxLOD"))
    {
        FArchive local_Ar(content["MaxLOD"], Ar);
        TStaticSerializer<decltype(this->MaxLOD)>::Deserialize(this->MaxLOD, local_Ar);
    }
    if (content.contains("PassLOD"))
    {
        FArchive local_Ar(content["PassLOD"], Ar);
        TStaticSerializer<decltype(this->PassLOD)>::Deserialize(this->PassLOD, local_Ar);
    }
    if (content.contains("ScreenSizeDenominator"))
    {
        FArchive local_Ar(content["ScreenSizeDenominator"], Ar);
        TStaticSerializer<decltype(this->ScreenSizeDenominator)>::Deserialize(this->ScreenSizeDenominator, local_Ar);
    }
    
}
