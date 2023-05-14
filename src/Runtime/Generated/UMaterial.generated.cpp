#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UMaterial::StaticClass_ = nullptr;
const NClass *UMaterial::GetClass() const 
{ 
    return UMaterial::StaticClass(); 
}
const NClass *UMaterial::StaticClass()
{
    return UMaterial::StaticClass_.get();
}

template<>
struct TClassRegistry<UMaterial>
{
    TClassRegistry(const std::string& InName)
    {
        UMaterial::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UMaterial>("UMaterial")
				   .AddDefaultConstructor()
				   .AddParentClass("UObject")
				   .AddDerivedClass("UMaterialInstance")
;
        UMaterial::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UMaterial> Dummy;
};
TClassRegistry<UMaterial> Dummy = TClassRegistry<UMaterial>("UMaterial");


