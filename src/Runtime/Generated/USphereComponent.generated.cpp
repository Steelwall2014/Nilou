#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::USphereComponent::StaticClass_ = nullptr;
const NClass *nilou::USphereComponent::GetClass() const 
{ 
    return nilou::USphereComponent::StaticClass(); 
}
const NClass *nilou::USphereComponent::StaticClass()
{
    return nilou::USphereComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::USphereComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::USphereComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::USphereComponent>();
		Mngr.AddBases<nilou::USphereComponent, nilou::UPrimitiveComponent>();
;
        nilou::USphereComponent::StaticClass_->Type = Type_of<nilou::USphereComponent>;
        nilou::USphereComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::USphereComponent>);
    }

    static TClassRegistry<nilou::USphereComponent> Dummy;
};
TClassRegistry<nilou::USphereComponent> Dummy = TClassRegistry<nilou::USphereComponent>("nilou::USphereComponent");



void nilou::USphereComponent::Serialize(FArchive& Ar)
{
    nilou::UPrimitiveComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::USphereComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::USphereComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UPrimitiveComponent::Deserialize(Ar);
}
