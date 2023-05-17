#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<UTextureRenderTarget>();
		Mngr.AddConstructor<UTextureRenderTarget>();
		Mngr.AddBases<UTextureRenderTarget, UTexture>();
;
        UTextureRenderTarget::StaticClass_->Type = Type_of<UTextureRenderTarget>;
        UTextureRenderTarget::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UTextureRenderTarget>);
    }

    static TClassRegistry<UTextureRenderTarget> Dummy;
};
TClassRegistry<UTextureRenderTarget> Dummy = TClassRegistry<UTextureRenderTarget>("UTextureRenderTarget");


