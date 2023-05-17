#include "D:/Nilou/src/Runtime/Rendering/Texture3D.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UTexture3D::StaticClass_ = nullptr;
const NClass *UTexture3D::GetClass() const 
{ 
    return UTexture3D::StaticClass(); 
}
const NClass *UTexture3D::StaticClass()
{
    return UTexture3D::StaticClass_.get();
}

template<>
struct TClassRegistry<UTexture3D>
{
    TClassRegistry(const std::string& InName)
    {
        UTexture3D::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UTexture3D>();
		Mngr.AddConstructor<UTexture3D>();
		Mngr.AddBases<UTexture3D, UTexture>();
;
        UTexture3D::StaticClass_->Type = Type_of<UTexture3D>;
        UTexture3D::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UTexture3D>);
    }

    static TClassRegistry<UTexture3D> Dummy;
};
TClassRegistry<UTexture3D> Dummy = TClassRegistry<UTexture3D>("UTexture3D");


