#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<UTextureRenderTargetCube>();
		Mngr.AddConstructor<UTextureRenderTargetCube>();
		Mngr.AddBases<UTextureRenderTargetCube, UTextureRenderTarget>();
;
        UTextureRenderTargetCube::StaticClass_->Type = Type_of<UTextureRenderTargetCube>;
        UTextureRenderTargetCube::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UTextureRenderTargetCube>);
    }

    static TClassRegistry<UTextureRenderTargetCube> Dummy;
};
TClassRegistry<UTextureRenderTargetCube> Dummy = TClassRegistry<UTextureRenderTargetCube>("UTextureRenderTargetCube");


