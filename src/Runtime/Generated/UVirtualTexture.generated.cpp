#include "D:/Nilou/src/Runtime/Rendering/VirtualTexture2D.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UVirtualTexture::StaticClass_ = nullptr;
const NClass *UVirtualTexture::GetClass() const 
{ 
    return UVirtualTexture::StaticClass(); 
}
const NClass *UVirtualTexture::StaticClass()
{
    return UVirtualTexture::StaticClass_.get();
}

template<>
struct TClassRegistry<UVirtualTexture>
{
    TClassRegistry(const std::string& InName)
    {
        UVirtualTexture::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UVirtualTexture>("UVirtualTexture")
				   .AddDefaultConstructor()
				   .AddParentClass("UTexture")
;
        UVirtualTexture::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UVirtualTexture> Dummy;
};
TClassRegistry<UVirtualTexture> Dummy = TClassRegistry<UVirtualTexture>("UVirtualTexture");


