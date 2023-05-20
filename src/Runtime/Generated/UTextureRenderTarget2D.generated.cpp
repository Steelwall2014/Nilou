#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UTextureRenderTarget2D::StaticClass_ = nullptr;
const NClass *nilou::UTextureRenderTarget2D::GetClass() const 
{ 
    return nilou::UTextureRenderTarget2D::StaticClass(); 
}
const NClass *nilou::UTextureRenderTarget2D::StaticClass()
{
    return nilou::UTextureRenderTarget2D::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UTextureRenderTarget2D>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UTextureRenderTarget2D::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UTextureRenderTarget2D>();
		Mngr.AddBases<nilou::UTextureRenderTarget2D, nilou::UTextureRenderTarget>();
;
        nilou::UTextureRenderTarget2D::StaticClass_->Type = Type_of<nilou::UTextureRenderTarget2D>;
        nilou::UTextureRenderTarget2D::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UTextureRenderTarget2D>);
    }

    static TClassRegistry<nilou::UTextureRenderTarget2D> Dummy;
};
TClassRegistry<nilou::UTextureRenderTarget2D> Dummy = TClassRegistry<nilou::UTextureRenderTarget2D>("nilou::UTextureRenderTarget2D");



void nilou::UTextureRenderTarget2D::Serialize(FArchive& Ar)
{
    nilou::UTextureRenderTarget::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UTextureRenderTarget2D";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UTextureRenderTarget2D::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UTextureRenderTarget::Deserialize(Ar);
}
