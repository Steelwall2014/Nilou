#include "D:/Nilou/src/Runtime/Rendering/Texture2D.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UTexture2D::StaticClass_ = nullptr;
const NClass *UTexture2D::GetClass() const 
{ 
    return UTexture2D::StaticClass(); 
}
const NClass *UTexture2D::StaticClass()
{
    return UTexture2D::StaticClass_.get();
}

template<>
struct TClassRegistry<UTexture2D>
{
    TClassRegistry(const std::string& InName)
    {
        UTexture2D::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UTexture2D>();
		Mngr.AddConstructor<UTexture2D>();
		Mngr.AddBases<UTexture2D, UTexture>();
;
        UTexture2D::StaticClass_->Type = Type_of<UTexture2D>;
        UTexture2D::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UTexture2D>);
    }

    static TClassRegistry<UTexture2D> Dummy;
};
TClassRegistry<UTexture2D> Dummy = TClassRegistry<UTexture2D>("UTexture2D");


