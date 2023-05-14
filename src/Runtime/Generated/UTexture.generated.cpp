#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UTexture::StaticClass_ = nullptr;
const NClass *UTexture::GetClass() const 
{ 
    return UTexture::StaticClass(); 
}
const NClass *UTexture::StaticClass()
{
    return UTexture::StaticClass_.get();
}

template<>
struct TClassRegistry<UTexture>
{
    TClassRegistry(const std::string& InName)
    {
        UTexture::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UTexture>("UTexture")
				   .AddDefaultConstructor()
				   .AddParentClass("UObject")
				   .AddDerivedClass("UTexture2D")
				   .AddDerivedClass("UTexture2DArray")
				   .AddDerivedClass("UTexture3D")
				   .AddDerivedClass("UTextureCube")
				   .AddDerivedClass("UTextureRenderTarget")
				   .AddDerivedClass("UVirtualTexture")
;
        UTexture::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UTexture> Dummy;
};
TClassRegistry<UTexture> Dummy = TClassRegistry<UTexture>("UTexture");


