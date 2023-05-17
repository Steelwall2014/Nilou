#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> AActor::StaticClass_ = nullptr;
const NClass *AActor::GetClass() const 
{ 
    return AActor::StaticClass(); 
}
const NClass *AActor::StaticClass()
{
    return AActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AActor>
{
    TClassRegistry(const std::string& InName)
    {
        AActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<AActor>();
		Mngr.AddConstructor<AActor>();
		Mngr.AddField<&AActor::ActorName>("ActorName");
		Mngr.AddBases<AActor, UObject>();
;
        AActor::StaticClass_->Type = Type_of<AActor>;
        AActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<AActor>);
    }

    static TClassRegistry<AActor> Dummy;
};
TClassRegistry<AActor> Dummy = TClassRegistry<AActor>("AActor");


