#include "D:/Nilou/src/Runtime/Rendering/TextureCube.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UTextureCube::StaticClass_ = nullptr;
const NClass *UTextureCube::GetClass() const 
{ 
    return UTextureCube::StaticClass(); 
}
const NClass *UTextureCube::StaticClass()
{
    return UTextureCube::StaticClass_.get();
}

template<>
struct TClassRegistry<UTextureCube>
{
    TClassRegistry(const std::string& InName)
    {
        UTextureCube::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UTextureCube>();
		Mngr.AddConstructor<UTextureCube>();
		Mngr.AddBases<UTextureCube, UTexture>();
;
        UTextureCube::StaticClass_->Type = Type_of<UTextureCube>;
        UTextureCube::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UTextureCube>);
    }

    static TClassRegistry<UTextureCube> Dummy;
};
TClassRegistry<UTextureCube> Dummy = TClassRegistry<UTextureCube>("UTextureCube");


