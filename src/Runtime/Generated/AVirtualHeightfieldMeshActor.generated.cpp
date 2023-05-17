#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> AVirtualHeightfieldMeshActor::StaticClass_ = nullptr;
const NClass *AVirtualHeightfieldMeshActor::GetClass() const 
{ 
    return AVirtualHeightfieldMeshActor::StaticClass(); 
}
const NClass *AVirtualHeightfieldMeshActor::StaticClass()
{
    return AVirtualHeightfieldMeshActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AVirtualHeightfieldMeshActor>
{
    TClassRegistry(const std::string& InName)
    {
        AVirtualHeightfieldMeshActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<AVirtualHeightfieldMeshActor>();
		Mngr.AddConstructor<AVirtualHeightfieldMeshActor>();
		Mngr.AddBases<AVirtualHeightfieldMeshActor, AActor>();
;
        AVirtualHeightfieldMeshActor::StaticClass_->Type = Type_of<AVirtualHeightfieldMeshActor>;
        AVirtualHeightfieldMeshActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<AVirtualHeightfieldMeshActor>);
    }

    static TClassRegistry<AVirtualHeightfieldMeshActor> Dummy;
};
TClassRegistry<AVirtualHeightfieldMeshActor> Dummy = TClassRegistry<AVirtualHeightfieldMeshActor>("AVirtualHeightfieldMeshActor");


