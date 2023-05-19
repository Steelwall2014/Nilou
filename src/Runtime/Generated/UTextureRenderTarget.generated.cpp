#include "D:/Nilou/src/Runtime/Rendering/SceneView.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UTextureRenderTarget::StaticClass_ = nullptr;
const NClass *nilou::UTextureRenderTarget::GetClass() const 
{ 
    return nilou::UTextureRenderTarget::StaticClass(); 
}
const NClass *nilou::UTextureRenderTarget::StaticClass()
{
    return nilou::UTextureRenderTarget::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UTextureRenderTarget>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UTextureRenderTarget::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UTextureRenderTarget>();
		Mngr.AddField<&nilou::UTextureRenderTarget::ClearColor>("ClearColor");
		Mngr.AddBases<nilou::UTextureRenderTarget, nilou::UTexture>();
;
        nilou::UTextureRenderTarget::StaticClass_->Type = Type_of<nilou::UTextureRenderTarget>;
        nilou::UTextureRenderTarget::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UTextureRenderTarget>);
    }

    static TClassRegistry<nilou::UTextureRenderTarget> Dummy;
};
TClassRegistry<nilou::UTextureRenderTarget> Dummy = TClassRegistry<nilou::UTextureRenderTarget>("nilou::UTextureRenderTarget");



void nilou::UTextureRenderTarget::Serialize(FArchive& Ar)
{
    nilou::UTexture::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UTextureRenderTarget";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["ClearColor"], Ar);
        TStaticSerializer<decltype(this->ClearColor)>::Serialize(this->ClearColor, local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::UTextureRenderTarget::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("ClearColor"))
    {
        FArchive local_Ar(content["ClearColor"], Ar);
        TStaticSerializer<decltype(this->ClearColor)>::Deserialize(this->ClearColor, local_Ar);
    }
    nilou::UTexture::Deserialize(Ar);
}
