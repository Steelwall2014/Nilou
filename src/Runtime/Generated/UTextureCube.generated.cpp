#include "D:/Nilou/src/Runtime/Rendering/TextureCube.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UTextureCube::StaticClass_ = nullptr;
const NClass *UTextureCube::GetClass() const 
{ 
    return UTextureCube::StaticClass(); 
}
const NClass *UTextureCube::StaticClass()
{
    return UTextureCube::StaticClass_.get();
}

template<>
struct TClassRegistry<UTextureCube>
{
    TClassRegistry(const std::string& InName)
    {
        UTextureCube::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UTextureCube>("UTextureCube")
				   .AddDefaultConstructor()
				   .AddParentClass("UTexture")
;
        UTextureCube::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UTextureCube> Dummy;
};
TClassRegistry<UTextureCube> Dummy = TClassRegistry<UTextureCube>("UTextureCube");


