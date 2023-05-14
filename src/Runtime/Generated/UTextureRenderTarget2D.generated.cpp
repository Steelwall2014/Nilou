#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UTextureRenderTarget2D::StaticClass_ = nullptr;
const NClass *UTextureRenderTarget2D::GetClass() const 
{ 
    return UTextureRenderTarget2D::StaticClass(); 
}
const NClass *UTextureRenderTarget2D::StaticClass()
{
    return UTextureRenderTarget2D::StaticClass_.get();
}

template<>
struct TClassRegistry<UTextureRenderTarget2D>
{
    TClassRegistry(const std::string& InName)
    {
        UTextureRenderTarget2D::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UTextureRenderTarget2D>("UTextureRenderTarget2D")
				   .AddDefaultConstructor()
				   .AddParentClass("UTextureRenderTarget")
;
        UTextureRenderTarget2D::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UTextureRenderTarget2D> Dummy;
};
TClassRegistry<UTextureRenderTarget2D> Dummy = TClassRegistry<UTextureRenderTarget2D>("UTextureRenderTarget2D");


