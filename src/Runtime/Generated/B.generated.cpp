#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> B::StaticClass_ = nullptr;
const NClass *B::GetClass() const 
{ 
    return B::StaticClass(); 
}
const NClass *B::StaticClass()
{
    return B::StaticClass_.get();
}

template<>
struct TClassRegistry<B>
{
    TClassRegistry(const std::string& InName)
    {
        B::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<B>();
		Mngr.AddField<&B::b>("b");
		Mngr.AddBases<B, A>();
;
        B::StaticClass_->Type = Type_of<B>;
        B::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<B>);
    }

    static TClassRegistry<B> Dummy;
};
TClassRegistry<B> Dummy = TClassRegistry<B>("B");


