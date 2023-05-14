#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UStaticMesh::StaticClass_ = nullptr;
const NClass *UStaticMesh::GetClass() const 
{ 
    return UStaticMesh::StaticClass(); 
}
const NClass *UStaticMesh::StaticClass()
{
    return UStaticMesh::StaticClass_.get();
}

template<>
struct TClassRegistry<UStaticMesh>
{
    TClassRegistry(const std::string& InName)
    {
        UStaticMesh::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UStaticMesh>("UStaticMesh")
				   .AddDefaultConstructor()
				   .AddParentClass("UObject")
;
        UStaticMesh::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UStaticMesh> Dummy;
};
TClassRegistry<UStaticMesh> Dummy = TClassRegistry<UStaticMesh>("UStaticMesh");


