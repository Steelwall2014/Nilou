#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UObject::StaticClass_ = nullptr;
const NClass *UObject::GetClass() const 
{ 
    return UObject::StaticClass(); 
}
const NClass *UObject::StaticClass()
{
    return UObject::StaticClass_.get();
}

template<>
struct TClassRegistry<UObject>
{
    TClassRegistry(const std::string& InName)
    {
        UObject::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UObject>();
;
        UObject::StaticClass_->Type = Type_of<UObject>;
        UObject::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UObject>);
    }

    static TClassRegistry<UObject> Dummy;
};
TClassRegistry<UObject> Dummy = TClassRegistry<UObject>("UObject");


