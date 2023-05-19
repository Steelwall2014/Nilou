#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::A::StaticClass_ = nullptr;
const NClass *nilou::A::GetClass() const 
{ 
    return nilou::A::StaticClass(); 
}
const NClass *nilou::A::StaticClass()
{
    return nilou::A::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::A>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::A::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::A>();
		Mngr.AddField<&nilou::A::a>("a");
		Mngr.AddBases<nilou::A, NObject>();
;
        nilou::A::StaticClass_->Type = Type_of<nilou::A>;
        nilou::A::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::A>);
    }

    static TClassRegistry<nilou::A> Dummy;
};
TClassRegistry<nilou::A> Dummy = TClassRegistry<nilou::A>("nilou::A");



void nilou::A::Serialize(FArchive& Ar)
{
    NObject::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::A";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["a"], Ar);
        TStaticSerializer<decltype(this->a)>::Serialize(this->a, local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::A::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("a"))
    {
        FArchive local_Ar(content["a"], Ar);
        TStaticSerializer<decltype(this->a)>::Deserialize(this->a, local_Ar);
    }
    NObject::Deserialize(Ar);
}
