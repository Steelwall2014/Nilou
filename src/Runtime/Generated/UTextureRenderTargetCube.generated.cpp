#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UTextureRenderTargetCube::StaticClass_ = nullptr;
const NClass *UTextureRenderTargetCube::GetClass() const 
{ 
    return UTextureRenderTargetCube::StaticClass(); 
}
const NClass *UTextureRenderTargetCube::StaticClass()
{
    return UTextureRenderTargetCube::StaticClass_.get();
}

template<>
struct TClassRegistry<UTextureRenderTargetCube>
{
    TClassRegistry(const std::string& InName)
    {
        UTextureRenderTargetCube::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UTextureRenderTargetCube>("UTextureRenderTargetCube")
				   .AddDefaultConstructor()
				   .AddParentClass("UTextureRenderTarget")
;
        UTextureRenderTargetCube::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UTextureRenderTargetCube> Dummy;
};
TClassRegistry<UTextureRenderTargetCube> Dummy = TClassRegistry<UTextureRenderTargetCube>("UTextureRenderTargetCube");


