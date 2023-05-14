#include "D:/Nilou/src/Runtime/Rendering/Texture2D.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UTexture2D::StaticClass_ = nullptr;
const NClass *UTexture2D::GetClass() const 
{ 
    return UTexture2D::StaticClass(); 
}
const NClass *UTexture2D::StaticClass()
{
    return UTexture2D::StaticClass_.get();
}

template<>
struct TClassRegistry<UTexture2D>
{
    TClassRegistry(const std::string& InName)
    {
        UTexture2D::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UTexture2D>("UTexture2D")
				   .AddDefaultConstructor()
				   .AddParentClass("UTexture")
;
        UTexture2D::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UTexture2D> Dummy;
};
TClassRegistry<UTexture2D> Dummy = TClassRegistry<UTexture2D>("UTexture2D");


