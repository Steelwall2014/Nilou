#include "D:/Nilou/src/Runtime/Framework/Common/Actor/CameraActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> ACameraActor::StaticClass_ = nullptr;
const NClass *ACameraActor::GetClass() const 
{ 
    return ACameraActor::StaticClass(); 
}
const NClass *ACameraActor::StaticClass()
{
    return ACameraActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ACameraActor>
{
    TClassRegistry(const std::string& InName)
    {
        ACameraActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<ACameraActor>();
		Mngr.AddConstructor<ACameraActor>();
		Mngr.AddBases<ACameraActor, AActor>();
;
        ACameraActor::StaticClass_->Type = Type_of<ACameraActor>;
        ACameraActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<ACameraActor>);
    }

    static TClassRegistry<ACameraActor> Dummy;
};
TClassRegistry<ACameraActor> Dummy = TClassRegistry<ACameraActor>("ACameraActor");


