#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LineBatchActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> ULineBatchComponent::StaticClass_ = nullptr;
const NClass *ULineBatchComponent::GetClass() const 
{ 
    return ULineBatchComponent::StaticClass(); 
}
const NClass *ULineBatchComponent::StaticClass()
{
    return ULineBatchComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<ULineBatchComponent>
{
    TClassRegistry(const std::string& InName)
    {
        ULineBatchComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<ULineBatchComponent>();
		Mngr.AddConstructor<ULineBatchComponent>();
		Mngr.AddBases<ULineBatchComponent, UPrimitiveComponent>();
;
        ULineBatchComponent::StaticClass_->Type = Type_of<ULineBatchComponent>;
        ULineBatchComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<ULineBatchComponent>);
    }

    static TClassRegistry<ULineBatchComponent> Dummy;
};
TClassRegistry<ULineBatchComponent> Dummy = TClassRegistry<ULineBatchComponent>("ULineBatchComponent");


