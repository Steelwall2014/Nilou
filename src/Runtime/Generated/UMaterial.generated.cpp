#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UMaterial::StaticClass_ = nullptr;
const NClass *nilou::UMaterial::GetClass() const 
{ 
    return nilou::UMaterial::StaticClass(); 
}
const NClass *nilou::UMaterial::StaticClass()
{
    return nilou::UMaterial::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UMaterial>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UMaterial::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UMaterial>();
		Mngr.AddField<&nilou::UMaterial::BlendState>("BlendState");
		Mngr.AddField<&nilou::UMaterial::DepthStencilState>("DepthStencilState");
		Mngr.AddField<&nilou::UMaterial::Name>("Name");
		Mngr.AddField<&nilou::UMaterial::RasterizerState>("RasterizerState");
		Mngr.AddField<&nilou::UMaterial::ShaderVirtualPath>("ShaderVirtualPath");
		Mngr.AddField<&nilou::UMaterial::ShadingModel>("ShadingModel");
		Mngr.AddField<&nilou::UMaterial::StencilRefValue>("StencilRefValue");
		Mngr.AddField<&nilou::UMaterial::Textures>("Textures");
		Mngr.AddBases<nilou::UMaterial, nilou::NAsset>();
;
        nilou::UMaterial::StaticClass_->Type = Type_of<nilou::UMaterial>;
        nilou::UMaterial::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UMaterial>);
    }

    static TClassRegistry<nilou::UMaterial> Dummy;
};
TClassRegistry<nilou::UMaterial> Dummy = TClassRegistry<nilou::UMaterial>("nilou::UMaterial");



void nilou::UMaterial::Serialize(FArchive& Ar)
{
    nilou::NAsset::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UMaterial";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["BlendState"], Ar);
        this->BlendState.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["DepthStencilState"], Ar);
        this->DepthStencilState.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["Name"], Ar);
        TStaticSerializer<decltype(this->Name)>::Serialize(this->Name, local_Ar);
    }
    {
        FArchive local_Ar(content["RasterizerState"], Ar);
        this->RasterizerState.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["ShaderVirtualPath"], Ar);
        TStaticSerializer<decltype(this->ShaderVirtualPath)>::Serialize(this->ShaderVirtualPath, local_Ar);
    }
    {
        FArchive local_Ar(content["ShadingModel"], Ar);
        TStaticSerializer<decltype(this->ShadingModel)>::Serialize(this->ShadingModel, local_Ar);
    }
    {
        FArchive local_Ar(content["StencilRefValue"], Ar);
        TStaticSerializer<decltype(this->StencilRefValue)>::Serialize(this->StencilRefValue, local_Ar);
    }
    {
        FArchive local_Ar(content["Textures"], Ar);
        TStaticSerializer<decltype(this->Textures)>::Serialize(this->Textures, local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::UMaterial::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("BlendState"))
    {
        FArchive local_Ar(content["BlendState"], Ar);
        this->BlendState.Deserialize(local_Ar);
    }
    if (content.contains("DepthStencilState"))
    {
        FArchive local_Ar(content["DepthStencilState"], Ar);
        this->DepthStencilState.Deserialize(local_Ar);
    }
    if (content.contains("Name"))
    {
        FArchive local_Ar(content["Name"], Ar);
        TStaticSerializer<decltype(this->Name)>::Deserialize(this->Name, local_Ar);
    }
    if (content.contains("RasterizerState"))
    {
        FArchive local_Ar(content["RasterizerState"], Ar);
        this->RasterizerState.Deserialize(local_Ar);
    }
    if (content.contains("ShaderVirtualPath"))
    {
        FArchive local_Ar(content["ShaderVirtualPath"], Ar);
        TStaticSerializer<decltype(this->ShaderVirtualPath)>::Deserialize(this->ShaderVirtualPath, local_Ar);
    }
    if (content.contains("ShadingModel"))
    {
        FArchive local_Ar(content["ShadingModel"], Ar);
        TStaticSerializer<decltype(this->ShadingModel)>::Deserialize(this->ShadingModel, local_Ar);
    }
    if (content.contains("StencilRefValue"))
    {
        FArchive local_Ar(content["StencilRefValue"], Ar);
        TStaticSerializer<decltype(this->StencilRefValue)>::Deserialize(this->StencilRefValue, local_Ar);
    }
    if (content.contains("Textures"))
    {
        FArchive local_Ar(content["Textures"], Ar);
        TStaticSerializer<decltype(this->Textures)>::Deserialize(this->Textures, local_Ar);
    }
    nilou::NAsset::Deserialize(Ar);
}
