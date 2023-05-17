#include "D:/Nilou/src/Runtime/Rendering/VirtualTexture2D.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<UVirtualTexture>();
		Mngr.AddConstructor<UVirtualTexture>();
		Mngr.AddBases<UVirtualTexture, UTexture>();
;
        UVirtualTexture::StaticClass_->Type = Type_of<UVirtualTexture>;
        UVirtualTexture::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UVirtualTexture>);
    }

    static TClassRegistry<UVirtualTexture> Dummy;
};
TClassRegistry<UVirtualTexture> Dummy = TClassRegistry<UVirtualTexture>("UVirtualTexture");


