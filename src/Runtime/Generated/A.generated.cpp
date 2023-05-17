#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> A::StaticClass_ = nullptr;
const NClass *A::GetClass() const 
{ 
    return A::StaticClass(); 
}
const NClass *A::StaticClass()
{
    return A::StaticClass_.get();
}

template<>
struct TClassRegistry<A>
{
    TClassRegistry(const std::string& InName)
    {
        A::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<A>();
		Mngr.AddField<&A::a>("a");
		Mngr.AddBases<A, UObject>();
;
        A::StaticClass_->Type = Type_of<A>;
        A::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<A>);
    }

    static TClassRegistry<A> Dummy;
};
TClassRegistry<A> Dummy = TClassRegistry<A>("A");


