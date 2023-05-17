#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<UTexture>();
		Mngr.AddConstructor<UTexture>();
		Mngr.AddBases<UTexture, UObject>();
;
        UTexture::StaticClass_->Type = Type_of<UTexture>;
        UTexture::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UTexture>);
    }

    static TClassRegistry<UTexture> Dummy;
};
TClassRegistry<UTexture> Dummy = TClassRegistry<UTexture>("UTexture");


