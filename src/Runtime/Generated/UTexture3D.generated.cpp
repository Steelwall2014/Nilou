#include "D:/Nilou/src/Runtime/Rendering/Texture3D.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UTexture3D::StaticClass_ = nullptr;
const NClass *UTexture3D::GetClass() const 
{ 
    return UTexture3D::StaticClass(); 
}
const NClass *UTexture3D::StaticClass()
{
    return UTexture3D::StaticClass_.get();
}

template<>
struct TClassRegistry<UTexture3D>
{
    TClassRegistry(const std::string& InName)
    {
        UTexture3D::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UTexture3D>("UTexture3D")
				   .AddDefaultConstructor()
				   .AddParentClass("UTexture")
;
        UTexture3D::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UTexture3D> Dummy;
};
TClassRegistry<UTexture3D> Dummy = TClassRegistry<UTexture3D>("UTexture3D");


