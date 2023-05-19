#include "D:/Nilou/src/Runtime/Rendering/VirtualTexture2D.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UVirtualTexture::StaticClass_ = nullptr;
const NClass *nilou::UVirtualTexture::GetClass() const 
{ 
    return nilou::UVirtualTexture::StaticClass(); 
}
const NClass *nilou::UVirtualTexture::StaticClass()
{
    return nilou::UVirtualTexture::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UVirtualTexture>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UVirtualTexture::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UVirtualTexture>();
		Mngr.AddBases<nilou::UVirtualTexture, nilou::UTexture>();
;
        nilou::UVirtualTexture::StaticClass_->Type = Type_of<nilou::UVirtualTexture>;
        nilou::UVirtualTexture::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UVirtualTexture>);
    }

    static TClassRegistry<nilou::UVirtualTexture> Dummy;
};
TClassRegistry<nilou::UVirtualTexture> Dummy = TClassRegistry<nilou::UVirtualTexture>("nilou::UVirtualTexture");



void nilou::UVirtualTexture::Serialize(FArchive& Ar)
{
    nilou::UTexture::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UVirtualTexture";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UVirtualTexture::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UTexture::Deserialize(Ar);
}
