#include "D:/Nilou/src/Runtime/Rendering/Texture2D.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UTexture2D::StaticClass_ = nullptr;
const NClass *nilou::UTexture2D::GetClass() const 
{ 
    return nilou::UTexture2D::StaticClass(); 
}
const NClass *nilou::UTexture2D::StaticClass()
{
    return nilou::UTexture2D::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UTexture2D>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UTexture2D::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UTexture2D>();
		Mngr.AddBases<nilou::UTexture2D, nilou::UTexture>();
;
        nilou::UTexture2D::StaticClass_->Type = Type_of<nilou::UTexture2D>;
        nilou::UTexture2D::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UTexture2D>);
    }

    static TClassRegistry<nilou::UTexture2D> Dummy;
};
TClassRegistry<nilou::UTexture2D> Dummy = TClassRegistry<nilou::UTexture2D>("nilou::UTexture2D");



void nilou::UTexture2D::Serialize(FArchive& Ar)
{
    nilou::UTexture::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UTexture2D";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UTexture2D::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UTexture::Deserialize(Ar);
}
