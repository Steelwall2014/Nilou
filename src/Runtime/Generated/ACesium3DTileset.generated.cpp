#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> ACesium3DTileset::StaticClass_ = nullptr;
const NClass *ACesium3DTileset::GetClass() const 
{ 
    return ACesium3DTileset::StaticClass(); 
}
const NClass *ACesium3DTileset::StaticClass()
{
    return ACesium3DTileset::StaticClass_.get();
}

template<>
struct TClassRegistry<ACesium3DTileset>
{
    TClassRegistry(const std::string& InName)
    {
        ACesium3DTileset::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<ACesium3DTileset>();
		Mngr.AddConstructor<ACesium3DTileset>();
		Mngr.AddBases<ACesium3DTileset, AActor>();
;
        ACesium3DTileset::StaticClass_->Type = Type_of<ACesium3DTileset>;
        ACesium3DTileset::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<ACesium3DTileset>);
    }

    static TClassRegistry<ACesium3DTileset> Dummy;
};
TClassRegistry<ACesium3DTileset> Dummy = TClassRegistry<ACesium3DTileset>("ACesium3DTileset");


