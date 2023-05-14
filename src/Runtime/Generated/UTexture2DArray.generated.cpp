#include "D:/Nilou/src/Runtime/Rendering/Texture2DArray.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UTexture2DArray::StaticClass_ = nullptr;
const NClass *UTexture2DArray::GetClass() const 
{ 
    return UTexture2DArray::StaticClass(); 
}
const NClass *UTexture2DArray::StaticClass()
{
    return UTexture2DArray::StaticClass_.get();
}

template<>
struct TClassRegistry<UTexture2DArray>
{
    TClassRegistry(const std::string& InName)
    {
        UTexture2DArray::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UTexture2DArray>("UTexture2DArray")
				   .AddDefaultConstructor()
				   .AddParentClass("UTexture")
;
        UTexture2DArray::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UTexture2DArray> Dummy;
};
TClassRegistry<UTexture2DArray> Dummy = TClassRegistry<UTexture2DArray>("UTexture2DArray");


