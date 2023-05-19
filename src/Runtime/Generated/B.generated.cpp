#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::B::StaticClass_ = nullptr;
const NClass *nilou::B::GetClass() const 
{ 
    return nilou::B::StaticClass(); 
}
const NClass *nilou::B::StaticClass()
{
    return nilou::B::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::B>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::B::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::B>();
		Mngr.AddField<&nilou::B::b>("b");
		Mngr.AddBases<nilou::B, nilou::A>();
;
        nilou::B::StaticClass_->Type = Type_of<nilou::B>;
        nilou::B::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::B>);
    }

    static TClassRegistry<nilou::B> Dummy;
};
TClassRegistry<nilou::B> Dummy = TClassRegistry<nilou::B>("nilou::B");



void nilou::B::Serialize(FArchive& Ar)
{
    nilou::A::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::B";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["b"], Ar);
        TStaticSerializer<decltype(this->b)>::Serialize(this->b, local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::B::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("b"))
    {
        FArchive local_Ar(content["b"], Ar);
        TStaticSerializer<decltype(this->b)>::Deserialize(this->b, local_Ar);
    }
    nilou::A::Deserialize(Ar);
}
