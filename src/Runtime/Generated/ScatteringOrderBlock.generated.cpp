#include "D:/Nilou/src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ScatteringOrderBlock::StaticClass_ = nullptr;
const NClass *nilou::ScatteringOrderBlock::GetClass() const 
{ 
    return nilou::ScatteringOrderBlock::StaticClass(); 
}
const NClass *nilou::ScatteringOrderBlock::StaticClass()
{
    return nilou::ScatteringOrderBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ScatteringOrderBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ScatteringOrderBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ScatteringOrderBlock>();
		Mngr.AddField<&nilou::ScatteringOrderBlock::ScatteringOrder>("ScatteringOrder");
		Mngr.AddMethod<&nilou::ScatteringOrderBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::ScatteringOrderBlock::Serialize>("Serialize");
;
        nilou::ScatteringOrderBlock::StaticClass_->Type = Type_of<nilou::ScatteringOrderBlock>;
        nilou::ScatteringOrderBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ScatteringOrderBlock>);
    }

    static TClassRegistry<nilou::ScatteringOrderBlock> Dummy;
};
TClassRegistry<nilou::ScatteringOrderBlock> Dummy = TClassRegistry<nilou::ScatteringOrderBlock>("nilou::ScatteringOrderBlock");



void nilou::ScatteringOrderBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["ScatteringOrder"], Ar);
        TStaticSerializer<decltype(this->ScatteringOrder)>::Serialize(this->ScatteringOrder, local_Ar);
    }
}

void nilou::ScatteringOrderBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("ScatteringOrder"))
    {
        FArchive local_Ar(content["ScatteringOrder"], Ar);
        TStaticSerializer<decltype(this->ScatteringOrder)>::Deserialize(this->ScatteringOrder, local_Ar);
    }
    
}
