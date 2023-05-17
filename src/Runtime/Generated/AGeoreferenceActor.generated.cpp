#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> AGeoreferenceActor::StaticClass_ = nullptr;
const NClass *AGeoreferenceActor::GetClass() const 
{ 
    return AGeoreferenceActor::StaticClass(); 
}
const NClass *AGeoreferenceActor::StaticClass()
{
    return AGeoreferenceActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AGeoreferenceActor>
{
    TClassRegistry(const std::string& InName)
    {
        AGeoreferenceActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<AGeoreferenceActor>();
		Mngr.AddConstructor<AGeoreferenceActor>();
		Mngr.AddBases<AGeoreferenceActor, AActor>();
;
        AGeoreferenceActor::StaticClass_->Type = Type_of<AGeoreferenceActor>;
        AGeoreferenceActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<AGeoreferenceActor>);
    }

    static TClassRegistry<AGeoreferenceActor> Dummy;
};
TClassRegistry<AGeoreferenceActor> Dummy = TClassRegistry<AGeoreferenceActor>("AGeoreferenceActor");


