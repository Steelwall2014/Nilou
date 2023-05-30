#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UTextureRenderTargetCube::StaticClass_ = nullptr;
const NClass *nilou::UTextureRenderTargetCube::GetClass() const 
{ 
    return nilou::UTextureRenderTargetCube::StaticClass(); 
}
const NClass *nilou::UTextureRenderTargetCube::StaticClass()
{
    return nilou::UTextureRenderTargetCube::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UTextureRenderTargetCube>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UTextureRenderTargetCube::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UTextureRenderTargetCube>();
		Mngr.AddBases<nilou::UTextureRenderTargetCube, nilou::UTextureRenderTarget>();
;
        nilou::UTextureRenderTargetCube::StaticClass_->Type = Type_of<nilou::UTextureRenderTargetCube>;
        nilou::UTextureRenderTargetCube::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UTextureRenderTargetCube>);
    }

    static TClassRegistry<nilou::UTextureRenderTargetCube> Dummy;
};
TClassRegistry<nilou::UTextureRenderTargetCube> Dummy = TClassRegistry<nilou::UTextureRenderTargetCube>("nilou::UTextureRenderTargetCube");



void nilou::UTextureRenderTargetCube::Serialize(FArchive& Ar)
{
    nilou::UTextureRenderTarget::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UTextureRenderTargetCube";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UTextureRenderTargetCube::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UTextureRenderTarget::Deserialize(Ar);
}
