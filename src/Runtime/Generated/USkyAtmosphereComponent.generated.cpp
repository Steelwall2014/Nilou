#include "D:/Nilou/src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::USkyAtmosphereComponent::StaticClass_ = nullptr;
const NClass *nilou::USkyAtmosphereComponent::GetClass() const 
{ 
    return nilou::USkyAtmosphereComponent::StaticClass(); 
}
const NClass *nilou::USkyAtmosphereComponent::StaticClass()
{
    return nilou::USkyAtmosphereComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::USkyAtmosphereComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::USkyAtmosphereComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::USkyAtmosphereComponent>();
		Mngr.AddBases<nilou::USkyAtmosphereComponent, nilou::USceneComponent>();
;
        nilou::USkyAtmosphereComponent::StaticClass_->Type = Type_of<nilou::USkyAtmosphereComponent>;
        nilou::USkyAtmosphereComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::USkyAtmosphereComponent>);
    }

    static TClassRegistry<nilou::USkyAtmosphereComponent> Dummy;
};
TClassRegistry<nilou::USkyAtmosphereComponent> Dummy = TClassRegistry<nilou::USkyAtmosphereComponent>("nilou::USkyAtmosphereComponent");



void nilou::USkyAtmosphereComponent::Serialize(FArchive& Ar)
{
    nilou::USceneComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::USkyAtmosphereComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::USkyAtmosphereComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::USceneComponent::Deserialize(Ar);
}
