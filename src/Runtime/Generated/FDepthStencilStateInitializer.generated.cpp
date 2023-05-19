#include "D:/Nilou/src/Runtime/RHI/RHI.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FDepthStencilStateInitializer::StaticClass_ = nullptr;
const NClass *nilou::FDepthStencilStateInitializer::GetClass() const 
{ 
    return nilou::FDepthStencilStateInitializer::StaticClass(); 
}
const NClass *nilou::FDepthStencilStateInitializer::StaticClass()
{
    return nilou::FDepthStencilStateInitializer::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FDepthStencilStateInitializer>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FDepthStencilStateInitializer::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FDepthStencilStateInitializer>();
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::BackFaceDepthFailStencilOp>("BackFaceDepthFailStencilOp");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::BackFacePassStencilOp>("BackFacePassStencilOp");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::BackFaceStencilFailStencilOp>("BackFaceStencilFailStencilOp");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::BackFaceStencilTest>("BackFaceStencilTest");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::DepthTest>("DepthTest");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::FrontFaceDepthFailStencilOp>("FrontFaceDepthFailStencilOp");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::FrontFacePassStencilOp>("FrontFacePassStencilOp");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::FrontFaceStencilFailStencilOp>("FrontFaceStencilFailStencilOp");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::FrontFaceStencilTest>("FrontFaceStencilTest");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::StencilReadMask>("StencilReadMask");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::StencilWriteMask>("StencilWriteMask");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::bEnableBackFaceStencil>("bEnableBackFaceStencil");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::bEnableDepthWrite>("bEnableDepthWrite");
		Mngr.AddField<&nilou::FDepthStencilStateInitializer::bEnableFrontFaceStencil>("bEnableFrontFaceStencil");
;
        nilou::FDepthStencilStateInitializer::StaticClass_->Type = Type_of<nilou::FDepthStencilStateInitializer>;
        nilou::FDepthStencilStateInitializer::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FDepthStencilStateInitializer>);
    }

    static TClassRegistry<nilou::FDepthStencilStateInitializer> Dummy;
};
TClassRegistry<nilou::FDepthStencilStateInitializer> Dummy = TClassRegistry<nilou::FDepthStencilStateInitializer>("nilou::FDepthStencilStateInitializer");



void nilou::FDepthStencilStateInitializer::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["BackFaceDepthFailStencilOp"], Ar);
        TStaticSerializer<decltype(this->BackFaceDepthFailStencilOp)>::Serialize(this->BackFaceDepthFailStencilOp, local_Ar);
    }
    {
        FArchive local_Ar(content["BackFacePassStencilOp"], Ar);
        TStaticSerializer<decltype(this->BackFacePassStencilOp)>::Serialize(this->BackFacePassStencilOp, local_Ar);
    }
    {
        FArchive local_Ar(content["BackFaceStencilFailStencilOp"], Ar);
        TStaticSerializer<decltype(this->BackFaceStencilFailStencilOp)>::Serialize(this->BackFaceStencilFailStencilOp, local_Ar);
    }
    {
        FArchive local_Ar(content["BackFaceStencilTest"], Ar);
        TStaticSerializer<decltype(this->BackFaceStencilTest)>::Serialize(this->BackFaceStencilTest, local_Ar);
    }
    {
        FArchive local_Ar(content["DepthTest"], Ar);
        TStaticSerializer<decltype(this->DepthTest)>::Serialize(this->DepthTest, local_Ar);
    }
    {
        FArchive local_Ar(content["FrontFaceDepthFailStencilOp"], Ar);
        TStaticSerializer<decltype(this->FrontFaceDepthFailStencilOp)>::Serialize(this->FrontFaceDepthFailStencilOp, local_Ar);
    }
    {
        FArchive local_Ar(content["FrontFacePassStencilOp"], Ar);
        TStaticSerializer<decltype(this->FrontFacePassStencilOp)>::Serialize(this->FrontFacePassStencilOp, local_Ar);
    }
    {
        FArchive local_Ar(content["FrontFaceStencilFailStencilOp"], Ar);
        TStaticSerializer<decltype(this->FrontFaceStencilFailStencilOp)>::Serialize(this->FrontFaceStencilFailStencilOp, local_Ar);
    }
    {
        FArchive local_Ar(content["FrontFaceStencilTest"], Ar);
        TStaticSerializer<decltype(this->FrontFaceStencilTest)>::Serialize(this->FrontFaceStencilTest, local_Ar);
    }
    {
        FArchive local_Ar(content["StencilReadMask"], Ar);
        TStaticSerializer<decltype(this->StencilReadMask)>::Serialize(this->StencilReadMask, local_Ar);
    }
    {
        FArchive local_Ar(content["StencilWriteMask"], Ar);
        TStaticSerializer<decltype(this->StencilWriteMask)>::Serialize(this->StencilWriteMask, local_Ar);
    }
    {
        FArchive local_Ar(content["bEnableBackFaceStencil"], Ar);
        TStaticSerializer<decltype(this->bEnableBackFaceStencil)>::Serialize(this->bEnableBackFaceStencil, local_Ar);
    }
    {
        FArchive local_Ar(content["bEnableDepthWrite"], Ar);
        TStaticSerializer<decltype(this->bEnableDepthWrite)>::Serialize(this->bEnableDepthWrite, local_Ar);
    }
    {
        FArchive local_Ar(content["bEnableFrontFaceStencil"], Ar);
        TStaticSerializer<decltype(this->bEnableFrontFaceStencil)>::Serialize(this->bEnableFrontFaceStencil, local_Ar);
    }
}

void nilou::FDepthStencilStateInitializer::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("BackFaceDepthFailStencilOp"))
    {
        FArchive local_Ar(content["BackFaceDepthFailStencilOp"], Ar);
        TStaticSerializer<decltype(this->BackFaceDepthFailStencilOp)>::Deserialize(this->BackFaceDepthFailStencilOp, local_Ar);
    }
    if (content.contains("BackFacePassStencilOp"))
    {
        FArchive local_Ar(content["BackFacePassStencilOp"], Ar);
        TStaticSerializer<decltype(this->BackFacePassStencilOp)>::Deserialize(this->BackFacePassStencilOp, local_Ar);
    }
    if (content.contains("BackFaceStencilFailStencilOp"))
    {
        FArchive local_Ar(content["BackFaceStencilFailStencilOp"], Ar);
        TStaticSerializer<decltype(this->BackFaceStencilFailStencilOp)>::Deserialize(this->BackFaceStencilFailStencilOp, local_Ar);
    }
    if (content.contains("BackFaceStencilTest"))
    {
        FArchive local_Ar(content["BackFaceStencilTest"], Ar);
        TStaticSerializer<decltype(this->BackFaceStencilTest)>::Deserialize(this->BackFaceStencilTest, local_Ar);
    }
    if (content.contains("DepthTest"))
    {
        FArchive local_Ar(content["DepthTest"], Ar);
        TStaticSerializer<decltype(this->DepthTest)>::Deserialize(this->DepthTest, local_Ar);
    }
    if (content.contains("FrontFaceDepthFailStencilOp"))
    {
        FArchive local_Ar(content["FrontFaceDepthFailStencilOp"], Ar);
        TStaticSerializer<decltype(this->FrontFaceDepthFailStencilOp)>::Deserialize(this->FrontFaceDepthFailStencilOp, local_Ar);
    }
    if (content.contains("FrontFacePassStencilOp"))
    {
        FArchive local_Ar(content["FrontFacePassStencilOp"], Ar);
        TStaticSerializer<decltype(this->FrontFacePassStencilOp)>::Deserialize(this->FrontFacePassStencilOp, local_Ar);
    }
    if (content.contains("FrontFaceStencilFailStencilOp"))
    {
        FArchive local_Ar(content["FrontFaceStencilFailStencilOp"], Ar);
        TStaticSerializer<decltype(this->FrontFaceStencilFailStencilOp)>::Deserialize(this->FrontFaceStencilFailStencilOp, local_Ar);
    }
    if (content.contains("FrontFaceStencilTest"))
    {
        FArchive local_Ar(content["FrontFaceStencilTest"], Ar);
        TStaticSerializer<decltype(this->FrontFaceStencilTest)>::Deserialize(this->FrontFaceStencilTest, local_Ar);
    }
    if (content.contains("StencilReadMask"))
    {
        FArchive local_Ar(content["StencilReadMask"], Ar);
        TStaticSerializer<decltype(this->StencilReadMask)>::Deserialize(this->StencilReadMask, local_Ar);
    }
    if (content.contains("StencilWriteMask"))
    {
        FArchive local_Ar(content["StencilWriteMask"], Ar);
        TStaticSerializer<decltype(this->StencilWriteMask)>::Deserialize(this->StencilWriteMask, local_Ar);
    }
    if (content.contains("bEnableBackFaceStencil"))
    {
        FArchive local_Ar(content["bEnableBackFaceStencil"], Ar);
        TStaticSerializer<decltype(this->bEnableBackFaceStencil)>::Deserialize(this->bEnableBackFaceStencil, local_Ar);
    }
    if (content.contains("bEnableDepthWrite"))
    {
        FArchive local_Ar(content["bEnableDepthWrite"], Ar);
        TStaticSerializer<decltype(this->bEnableDepthWrite)>::Deserialize(this->bEnableDepthWrite, local_Ar);
    }
    if (content.contains("bEnableFrontFaceStencil"))
    {
        FArchive local_Ar(content["bEnableFrontFaceStencil"], Ar);
        TStaticSerializer<decltype(this->bEnableFrontFaceStencil)>::Deserialize(this->bEnableFrontFaceStencil, local_Ar);
    }
    
}
