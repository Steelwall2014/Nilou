#include "D:/Nilou/src/Runtime/Rendering/SceneView.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UTexture::StaticClass_ = nullptr;
const NClass *nilou::UTexture::GetClass() const 
{ 
    return nilou::UTexture::StaticClass(); 
}
const NClass *nilou::UTexture::StaticClass()
{
    return nilou::UTexture::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UTexture>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UTexture::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UTexture>();
		Mngr.AddField<&nilou::UTexture::ImageData>("ImageData");
		Mngr.AddField<&nilou::UTexture::Name>("Name");
		Mngr.AddField<&nilou::UTexture::NumMips>("NumMips");
		Mngr.AddField<&nilou::UTexture::TextureParams>("TextureParams");
		Mngr.AddBases<nilou::UTexture, nilou::NAsset>();
;
        nilou::UTexture::StaticClass_->Type = Type_of<nilou::UTexture>;
        nilou::UTexture::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UTexture>);
    }

    static TClassRegistry<nilou::UTexture> Dummy;
};
TClassRegistry<nilou::UTexture> Dummy = TClassRegistry<nilou::UTexture>("nilou::UTexture");



void nilou::UTexture::Serialize(FArchive& Ar)
{
    nilou::NAsset::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UTexture";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["ImageData"], Ar);
        this->ImageData.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["Name"], Ar);
        TStaticSerializer<decltype(this->Name)>::Serialize(this->Name, local_Ar);
    }
    {
        FArchive local_Ar(content["NumMips"], Ar);
        TStaticSerializer<decltype(this->NumMips)>::Serialize(this->NumMips, local_Ar);
    }
    {
        FArchive local_Ar(content["TextureParams"], Ar);
        this->TextureParams.Serialize(local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::UTexture::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("ImageData"))
    {
        FArchive local_Ar(content["ImageData"], Ar);
        this->ImageData.Deserialize(local_Ar);
    }
    if (content.contains("Name"))
    {
        FArchive local_Ar(content["Name"], Ar);
        TStaticSerializer<decltype(this->Name)>::Deserialize(this->Name, local_Ar);
    }
    if (content.contains("NumMips"))
    {
        FArchive local_Ar(content["NumMips"], Ar);
        TStaticSerializer<decltype(this->NumMips)>::Deserialize(this->NumMips, local_Ar);
    }
    if (content.contains("TextureParams"))
    {
        FArchive local_Ar(content["TextureParams"], Ar);
        this->TextureParams.Deserialize(local_Ar);
    }
    nilou::NAsset::Deserialize(Ar);
}
