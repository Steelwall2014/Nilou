#include "D:/Nilou/src/Runtime/RHI/RHI.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FBlendStateInitializer::FRenderTarget::StaticClass_ = nullptr;
const NClass *nilou::FBlendStateInitializer::FRenderTarget::GetClass() const 
{ 
    return nilou::FBlendStateInitializer::FRenderTarget::StaticClass(); 
}
const NClass *nilou::FBlendStateInitializer::FRenderTarget::StaticClass()
{
    return nilou::FBlendStateInitializer::FRenderTarget::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FBlendStateInitializer::FRenderTarget>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FBlendStateInitializer::FRenderTarget::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FBlendStateInitializer::FRenderTarget>();
		Mngr.AddField<&nilou::FBlendStateInitializer::FRenderTarget::AlphaBlendOp>("AlphaBlendOp");
		Mngr.AddField<&nilou::FBlendStateInitializer::FRenderTarget::AlphaDestBlend>("AlphaDestBlend");
		Mngr.AddField<&nilou::FBlendStateInitializer::FRenderTarget::AlphaSrcBlend>("AlphaSrcBlend");
		Mngr.AddField<&nilou::FBlendStateInitializer::FRenderTarget::ColorBlendOp>("ColorBlendOp");
		Mngr.AddField<&nilou::FBlendStateInitializer::FRenderTarget::ColorDestBlend>("ColorDestBlend");
		Mngr.AddField<&nilou::FBlendStateInitializer::FRenderTarget::ColorSrcBlend>("ColorSrcBlend");
		Mngr.AddField<&nilou::FBlendStateInitializer::FRenderTarget::ColorWriteMask>("ColorWriteMask");
;
        nilou::FBlendStateInitializer::FRenderTarget::StaticClass_->Type = Type_of<nilou::FBlendStateInitializer::FRenderTarget>;
        nilou::FBlendStateInitializer::FRenderTarget::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FBlendStateInitializer::FRenderTarget>);
    }

    static TClassRegistry<nilou::FBlendStateInitializer::FRenderTarget> Dummy;
};
TClassRegistry<nilou::FBlendStateInitializer::FRenderTarget> Dummy = TClassRegistry<nilou::FBlendStateInitializer::FRenderTarget>("nilou::FBlendStateInitializer::FRenderTarget");



void nilou::FBlendStateInitializer::FRenderTarget::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["AlphaBlendOp"], Ar);
        TStaticSerializer<decltype(this->AlphaBlendOp)>::Serialize(this->AlphaBlendOp, local_Ar);
    }
    {
        FArchive local_Ar(content["AlphaDestBlend"], Ar);
        TStaticSerializer<decltype(this->AlphaDestBlend)>::Serialize(this->AlphaDestBlend, local_Ar);
    }
    {
        FArchive local_Ar(content["AlphaSrcBlend"], Ar);
        TStaticSerializer<decltype(this->AlphaSrcBlend)>::Serialize(this->AlphaSrcBlend, local_Ar);
    }
    {
        FArchive local_Ar(content["ColorBlendOp"], Ar);
        TStaticSerializer<decltype(this->ColorBlendOp)>::Serialize(this->ColorBlendOp, local_Ar);
    }
    {
        FArchive local_Ar(content["ColorDestBlend"], Ar);
        TStaticSerializer<decltype(this->ColorDestBlend)>::Serialize(this->ColorDestBlend, local_Ar);
    }
    {
        FArchive local_Ar(content["ColorSrcBlend"], Ar);
        TStaticSerializer<decltype(this->ColorSrcBlend)>::Serialize(this->ColorSrcBlend, local_Ar);
    }
    {
        FArchive local_Ar(content["ColorWriteMask"], Ar);
        TStaticSerializer<decltype(this->ColorWriteMask)>::Serialize(this->ColorWriteMask, local_Ar);
    }
}

void nilou::FBlendStateInitializer::FRenderTarget::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("AlphaBlendOp"))
    {
        FArchive local_Ar(content["AlphaBlendOp"], Ar);
        TStaticSerializer<decltype(this->AlphaBlendOp)>::Deserialize(this->AlphaBlendOp, local_Ar);
    }
    if (content.contains("AlphaDestBlend"))
    {
        FArchive local_Ar(content["AlphaDestBlend"], Ar);
        TStaticSerializer<decltype(this->AlphaDestBlend)>::Deserialize(this->AlphaDestBlend, local_Ar);
    }
    if (content.contains("AlphaSrcBlend"))
    {
        FArchive local_Ar(content["AlphaSrcBlend"], Ar);
        TStaticSerializer<decltype(this->AlphaSrcBlend)>::Deserialize(this->AlphaSrcBlend, local_Ar);
    }
    if (content.contains("ColorBlendOp"))
    {
        FArchive local_Ar(content["ColorBlendOp"], Ar);
        TStaticSerializer<decltype(this->ColorBlendOp)>::Deserialize(this->ColorBlendOp, local_Ar);
    }
    if (content.contains("ColorDestBlend"))
    {
        FArchive local_Ar(content["ColorDestBlend"], Ar);
        TStaticSerializer<decltype(this->ColorDestBlend)>::Deserialize(this->ColorDestBlend, local_Ar);
    }
    if (content.contains("ColorSrcBlend"))
    {
        FArchive local_Ar(content["ColorSrcBlend"], Ar);
        TStaticSerializer<decltype(this->ColorSrcBlend)>::Deserialize(this->ColorSrcBlend, local_Ar);
    }
    if (content.contains("ColorWriteMask"))
    {
        FArchive local_Ar(content["ColorWriteMask"], Ar);
        TStaticSerializer<decltype(this->ColorWriteMask)>::Deserialize(this->ColorWriteMask, local_Ar);
    }
    
}
