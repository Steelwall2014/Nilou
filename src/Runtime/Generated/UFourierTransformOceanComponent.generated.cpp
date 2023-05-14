#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

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
        reflection::AddClass<UFourierTransformOceanComponent>("UFourierTransformOceanComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UPrimitiveComponent")
;
        UFourierTransformOceanComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UFourierTransformOceanComponent> Dummy;
};
TClassRegistry<UFourierTransformOceanComponent> Dummy = TClassRegistry<UFourierTransformOceanComponent>("UFourierTransformOceanComponent");


