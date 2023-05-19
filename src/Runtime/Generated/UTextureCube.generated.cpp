#include "D:/Nilou/src/Runtime/Rendering/TextureCube.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UTextureCube::StaticClass_ = nullptr;
const NClass *nilou::UTextureCube::GetClass() const 
{ 
    return nilou::UTextureCube::StaticClass(); 
}
const NClass *nilou::UTextureCube::StaticClass()
{
    return nilou::UTextureCube::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UTextureCube>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UTextureCube::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UTextureCube>();
		Mngr.AddBases<nilou::UTextureCube, nilou::UTexture>();
;
        nilou::UTextureCube::StaticClass_->Type = Type_of<nilou::UTextureCube>;
        nilou::UTextureCube::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UTextureCube>);
    }

    static TClassRegistry<nilou::UTextureCube> Dummy;
};
TClassRegistry<nilou::UTextureCube> Dummy = TClassRegistry<nilou::UTextureCube>("nilou::UTextureCube");



void nilou::UTextureCube::Serialize(FArchive& Ar)
{
    nilou::UTexture::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UTextureCube";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UTextureCube::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UTexture::Deserialize(Ar);
}
