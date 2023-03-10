#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
namespace nilou {
std::string UFourierTransformOceanComponent::GetClassName() { return "UFourierTransformOceanComponent"; }
EUClasses UFourierTransformOceanComponent::GetClassEnum() { return EUClasses::MC_UFourierTransformOceanComponent; }
const UClass *UFourierTransformOceanComponent::GetClass() { return UFourierTransformOceanComponent::StaticClass(); }
const UClass *UFourierTransformOceanComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UFourierTransformOceanComponent", EUClasses::MC_UFourierTransformOceanComponent);
	return StaticClass;
}
std::unique_ptr<UObject> UFourierTransformOceanComponent::CreateDefaultObject()
{
    return std::make_unique<UFourierTransformOceanComponent>();
}
}
