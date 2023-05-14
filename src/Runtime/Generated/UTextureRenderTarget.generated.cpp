#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UTextureRenderTarget::StaticClass_ = nullptr;
const NClass *UTextureRenderTarget::GetClass() const 
{ 
    return UTextureRenderTarget::StaticClass(); 
}
const NClass *UTextureRenderTarget::StaticClass()
{
    return UTextureRenderTarget::StaticClass_.get();
}

template<>
struct TClassRegistry<UTextureRenderTarget>
{
    TClassRegistry(const std::string& InName)
    {
        UTextureRenderTarget::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UTextureRenderTarget>("UTextureRenderTarget")
				   .AddDefaultConstructor()
				   .AddParentClass("UTexture")
				   .AddDerivedClass("UTextureRenderTarget2D")
				   .AddDerivedClass("UTextureRenderTargetCube")
;
        UTextureRenderTarget::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UTextureRenderTarget> Dummy;
};
TClassRegistry<UTextureRenderTarget> Dummy = TClassRegistry<UTextureRenderTarget>("UTextureRenderTarget");


