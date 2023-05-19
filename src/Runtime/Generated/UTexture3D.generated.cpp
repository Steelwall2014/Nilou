#include "D:/Nilou/src/Runtime/Rendering/Texture3D.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UTexture3D::StaticClass_ = nullptr;
const NClass *nilou::UTexture3D::GetClass() const 
{ 
    return nilou::UTexture3D::StaticClass(); 
}
const NClass *nilou::UTexture3D::StaticClass()
{
    return nilou::UTexture3D::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UTexture3D>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UTexture3D::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UTexture3D>();
		Mngr.AddBases<nilou::UTexture3D, nilou::UTexture>();
;
        nilou::UTexture3D::StaticClass_->Type = Type_of<nilou::UTexture3D>;
        nilou::UTexture3D::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UTexture3D>);
    }

    static TClassRegistry<nilou::UTexture3D> Dummy;
};
TClassRegistry<nilou::UTexture3D> Dummy = TClassRegistry<nilou::UTexture3D>("nilou::UTexture3D");



void nilou::UTexture3D::Serialize(FArchive& Ar)
{
    nilou::UTexture::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UTexture3D";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UTexture3D::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UTexture::Deserialize(Ar);
}
