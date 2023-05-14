#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Test.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

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
        reflection::AddClass<B>("B")
				   .AddDefaultConstructor()
				   .AddMemberVariable("b", &B::b)
				   .AddParentClass("A")
;
        B::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<B> Dummy;
};
TClassRegistry<B> Dummy = TClassRegistry<B>("B");


