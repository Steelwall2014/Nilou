#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<UStaticMesh>();
		Mngr.AddConstructor<UStaticMesh>();
		Mngr.AddBases<UStaticMesh, UObject>();
;
        UStaticMesh::StaticClass_->Type = Type_of<UStaticMesh>;
        UStaticMesh::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UStaticMesh>);
    }

    static TClassRegistry<UStaticMesh> Dummy;
};
TClassRegistry<UStaticMesh> Dummy = TClassRegistry<UStaticMesh>("UStaticMesh");


