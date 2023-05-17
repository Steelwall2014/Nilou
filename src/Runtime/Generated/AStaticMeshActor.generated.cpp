#include "D:/Nilou/src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> AStaticMeshActor::StaticClass_ = nullptr;
const NClass *AStaticMeshActor::GetClass() const 
{ 
    return AStaticMeshActor::StaticClass(); 
}
const NClass *AStaticMeshActor::StaticClass()
{
    return AStaticMeshActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AStaticMeshActor>
{
    TClassRegistry(const std::string& InName)
    {
        AStaticMeshActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<AStaticMeshActor>();
		Mngr.AddConstructor<AStaticMeshActor>();
		Mngr.AddBases<AStaticMeshActor, AActor>();
;
        AStaticMeshActor::StaticClass_->Type = Type_of<AStaticMeshActor>;
        AStaticMeshActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<AStaticMeshActor>);
    }

    static TClassRegistry<AStaticMeshActor> Dummy;
};
TClassRegistry<AStaticMeshActor> Dummy = TClassRegistry<AStaticMeshActor>("AStaticMeshActor");


