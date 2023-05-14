#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Test.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

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
        reflection::AddClass<A>("A")
				   .AddDefaultConstructor()
				   .AddMemberVariable("a", &A::a)
				   .AddParentClass("UObject")
				   .AddDerivedClass("B")
;
        A::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<A> Dummy;
};
TClassRegistry<A> Dummy = TClassRegistry<A>("A");


