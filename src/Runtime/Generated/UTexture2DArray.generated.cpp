#include "D:/Nilou/src/Runtime/Rendering/Texture2DArray.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UTexture2DArray::StaticClass_ = nullptr;
const NClass *UTexture2DArray::GetClass() const 
{ 
    return UTexture2DArray::StaticClass(); 
}
const NClass *UTexture2DArray::StaticClass()
{
    return UTexture2DArray::StaticClass_.get();
}

template<>
struct TClassRegistry<UTexture2DArray>
{
    TClassRegistry(const std::string& InName)
    {
        UTexture2DArray::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UTexture2DArray>();
		Mngr.AddConstructor<UTexture2DArray>();
		Mngr.AddBases<UTexture2DArray, UTexture>();
;
        UTexture2DArray::StaticClass_->Type = Type_of<UTexture2DArray>;
        UTexture2DArray::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UTexture2DArray>);
    }

    static TClassRegistry<UTexture2DArray> Dummy;
};
TClassRegistry<UTexture2DArray> Dummy = TClassRegistry<UTexture2DArray>("UTexture2DArray");


