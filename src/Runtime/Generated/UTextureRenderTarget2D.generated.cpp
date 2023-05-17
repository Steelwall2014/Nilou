#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<UTextureRenderTarget2D>();
		Mngr.AddConstructor<UTextureRenderTarget2D>();
		Mngr.AddBases<UTextureRenderTarget2D, UTextureRenderTarget>();
;
        UTextureRenderTarget2D::StaticClass_->Type = Type_of<UTextureRenderTarget2D>;
        UTextureRenderTarget2D::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UTextureRenderTarget2D>);
    }

    static TClassRegistry<UTextureRenderTarget2D> Dummy;
};
TClassRegistry<UTextureRenderTarget2D> Dummy = TClassRegistry<UTextureRenderTarget2D>("UTextureRenderTarget2D");


