#include "D:/Nilou/src/Runtime/Rendering/Texture2DArray.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UTexture2DArray::StaticClass_ = nullptr;
const NClass *nilou::UTexture2DArray::GetClass() const 
{ 
    return nilou::UTexture2DArray::StaticClass(); 
}
const NClass *nilou::UTexture2DArray::StaticClass()
{
    return nilou::UTexture2DArray::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UTexture2DArray>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UTexture2DArray::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UTexture2DArray>();
		Mngr.AddBases<nilou::UTexture2DArray, nilou::UTexture>();
;
        nilou::UTexture2DArray::StaticClass_->Type = Type_of<nilou::UTexture2DArray>;
        nilou::UTexture2DArray::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UTexture2DArray>);
    }

    static TClassRegistry<nilou::UTexture2DArray> Dummy;
};
TClassRegistry<nilou::UTexture2DArray> Dummy = TClassRegistry<nilou::UTexture2DArray>("nilou::UTexture2DArray");



void nilou::UTexture2DArray::Serialize(FArchive& Ar)
{
    nilou::UTexture::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UTexture2DArray";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UTexture2DArray::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UTexture::Deserialize(Ar);
}
