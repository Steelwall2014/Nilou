#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UMaterialInstance::StaticClass_ = nullptr;
const NClass *UMaterialInstance::GetClass() const 
{ 
    return UMaterialInstance::StaticClass(); 
}
const NClass *UMaterialInstance::StaticClass()
{
    return UMaterialInstance::StaticClass_.get();
}

template<>
struct TClassRegistry<UMaterialInstance>
{
    TClassRegistry(const std::string& InName)
    {
        UMaterialInstance::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UMaterialInstance>("UMaterialInstance")
				   .AddDefaultConstructor()
				   .AddParentClass("UMaterial")
;
        UMaterialInstance::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UMaterialInstance> Dummy;
};
TClassRegistry<UMaterialInstance> Dummy = TClassRegistry<UMaterialInstance>("UMaterialInstance");


