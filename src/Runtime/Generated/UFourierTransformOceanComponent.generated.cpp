#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UFourierTransformOceanComponent::StaticClass_ = nullptr;
const NClass *UFourierTransformOceanComponent::GetClass() const 
{ 
    return UFourierTransformOceanComponent::StaticClass(); 
}
const NClass *UFourierTransformOceanComponent::StaticClass()
{
    return UFourierTransformOceanComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UFourierTransformOceanComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UFourierTransformOceanComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UFourierTransformOceanComponent>();
		Mngr.AddConstructor<UFourierTransformOceanComponent>();
		Mngr.AddBases<UFourierTransformOceanComponent, UPrimitiveComponent>();
;
        UFourierTransformOceanComponent::StaticClass_->Type = Type_of<UFourierTransformOceanComponent>;
        UFourierTransformOceanComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UFourierTransformOceanComponent>);
    }

    static TClassRegistry<UFourierTransformOceanComponent> Dummy;
};
TClassRegistry<UFourierTransformOceanComponent> Dummy = TClassRegistry<UFourierTransformOceanComponent>("UFourierTransformOceanComponent");


