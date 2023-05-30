#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FViewShaderParameters::StaticClass_ = nullptr;
const NClass *nilou::FViewShaderParameters::GetClass() const 
{ 
    return nilou::FViewShaderParameters::StaticClass(); 
}
const NClass *nilou::FViewShaderParameters::StaticClass()
{
    return nilou::FViewShaderParameters::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FViewShaderParameters>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FViewShaderParameters::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FViewShaderParameters>();
		Mngr.AddField<&nilou::FViewShaderParameters::AbsWorldToClip>("AbsWorldToClip");
		Mngr.AddField<&nilou::FViewShaderParameters::CameraDirection>("CameraDirection");
		Mngr.AddField<&nilou::FViewShaderParameters::CameraFarClipDist>("CameraFarClipDist");
		Mngr.AddField<&nilou::FViewShaderParameters::CameraNearClipDist>("CameraNearClipDist");
		Mngr.AddField<&nilou::FViewShaderParameters::CameraPosition>("CameraPosition");
		Mngr.AddField<&nilou::FViewShaderParameters::CameraResolution>("CameraResolution");
		Mngr.AddField<&nilou::FViewShaderParameters::CameraVerticalFieldOfView>("CameraVerticalFieldOfView");
		Mngr.AddField<&nilou::FViewShaderParameters::ClipToView>("ClipToView");
		Mngr.AddField<&nilou::FViewShaderParameters::FrustumPlanes>("FrustumPlanes");
		Mngr.AddField<&nilou::FViewShaderParameters::RelClipToWorld>("RelClipToWorld");
		Mngr.AddField<&nilou::FViewShaderParameters::RelWorldToClip>("RelWorldToClip");
		Mngr.AddField<&nilou::FViewShaderParameters::RelWorldToView>("RelWorldToView");
		Mngr.AddField<&nilou::FViewShaderParameters::ViewToClip>("ViewToClip");
		Mngr.AddMethod<&nilou::FViewShaderParameters::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FViewShaderParameters::Serialize>("Serialize");
;
        nilou::FViewShaderParameters::StaticClass_->Type = Type_of<nilou::FViewShaderParameters>;
        nilou::FViewShaderParameters::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FViewShaderParameters>);
    }

    static TClassRegistry<nilou::FViewShaderParameters> Dummy;
};
TClassRegistry<nilou::FViewShaderParameters> Dummy = TClassRegistry<nilou::FViewShaderParameters>("nilou::FViewShaderParameters");



void nilou::FViewShaderParameters::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["AbsWorldToClip"], Ar);
        TStaticSerializer<decltype(this->AbsWorldToClip)>::Serialize(this->AbsWorldToClip, local_Ar);
    }
    {
        FArchive local_Ar(content["CameraDirection"], Ar);
        TStaticSerializer<decltype(this->CameraDirection)>::Serialize(this->CameraDirection, local_Ar);
    }
    {
        FArchive local_Ar(content["CameraFarClipDist"], Ar);
        TStaticSerializer<decltype(this->CameraFarClipDist)>::Serialize(this->CameraFarClipDist, local_Ar);
    }
    {
        FArchive local_Ar(content["CameraNearClipDist"], Ar);
        TStaticSerializer<decltype(this->CameraNearClipDist)>::Serialize(this->CameraNearClipDist, local_Ar);
    }
    {
        FArchive local_Ar(content["CameraPosition"], Ar);
        TStaticSerializer<decltype(this->CameraPosition)>::Serialize(this->CameraPosition, local_Ar);
    }
    {
        FArchive local_Ar(content["CameraResolution"], Ar);
        TStaticSerializer<decltype(this->CameraResolution)>::Serialize(this->CameraResolution, local_Ar);
    }
    {
        FArchive local_Ar(content["CameraVerticalFieldOfView"], Ar);
        TStaticSerializer<decltype(this->CameraVerticalFieldOfView)>::Serialize(this->CameraVerticalFieldOfView, local_Ar);
    }
    {
        FArchive local_Ar(content["ClipToView"], Ar);
        TStaticSerializer<decltype(this->ClipToView)>::Serialize(this->ClipToView, local_Ar);
    }
    {
        FArchive local_Ar(content["FrustumPlanes"], Ar);
        TStaticSerializer<decltype(this->FrustumPlanes)>::Serialize(this->FrustumPlanes, local_Ar);
    }
    {
        FArchive local_Ar(content["RelClipToWorld"], Ar);
        TStaticSerializer<decltype(this->RelClipToWorld)>::Serialize(this->RelClipToWorld, local_Ar);
    }
    {
        FArchive local_Ar(content["RelWorldToClip"], Ar);
        TStaticSerializer<decltype(this->RelWorldToClip)>::Serialize(this->RelWorldToClip, local_Ar);
    }
    {
        FArchive local_Ar(content["RelWorldToView"], Ar);
        TStaticSerializer<decltype(this->RelWorldToView)>::Serialize(this->RelWorldToView, local_Ar);
    }
    {
        FArchive local_Ar(content["ViewToClip"], Ar);
        TStaticSerializer<decltype(this->ViewToClip)>::Serialize(this->ViewToClip, local_Ar);
    }
}

void nilou::FViewShaderParameters::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("AbsWorldToClip"))
    {
        FArchive local_Ar(content["AbsWorldToClip"], Ar);
        TStaticSerializer<decltype(this->AbsWorldToClip)>::Deserialize(this->AbsWorldToClip, local_Ar);
    }
    if (content.contains("CameraDirection"))
    {
        FArchive local_Ar(content["CameraDirection"], Ar);
        TStaticSerializer<decltype(this->CameraDirection)>::Deserialize(this->CameraDirection, local_Ar);
    }
    if (content.contains("CameraFarClipDist"))
    {
        FArchive local_Ar(content["CameraFarClipDist"], Ar);
        TStaticSerializer<decltype(this->CameraFarClipDist)>::Deserialize(this->CameraFarClipDist, local_Ar);
    }
    if (content.contains("CameraNearClipDist"))
    {
        FArchive local_Ar(content["CameraNearClipDist"], Ar);
        TStaticSerializer<decltype(this->CameraNearClipDist)>::Deserialize(this->CameraNearClipDist, local_Ar);
    }
    if (content.contains("CameraPosition"))
    {
        FArchive local_Ar(content["CameraPosition"], Ar);
        TStaticSerializer<decltype(this->CameraPosition)>::Deserialize(this->CameraPosition, local_Ar);
    }
    if (content.contains("CameraResolution"))
    {
        FArchive local_Ar(content["CameraResolution"], Ar);
        TStaticSerializer<decltype(this->CameraResolution)>::Deserialize(this->CameraResolution, local_Ar);
    }
    if (content.contains("CameraVerticalFieldOfView"))
    {
        FArchive local_Ar(content["CameraVerticalFieldOfView"], Ar);
        TStaticSerializer<decltype(this->CameraVerticalFieldOfView)>::Deserialize(this->CameraVerticalFieldOfView, local_Ar);
    }
    if (content.contains("ClipToView"))
    {
        FArchive local_Ar(content["ClipToView"], Ar);
        TStaticSerializer<decltype(this->ClipToView)>::Deserialize(this->ClipToView, local_Ar);
    }
    if (content.contains("FrustumPlanes"))
    {
        FArchive local_Ar(content["FrustumPlanes"], Ar);
        TStaticSerializer<decltype(this->FrustumPlanes)>::Deserialize(this->FrustumPlanes, local_Ar);
    }
    if (content.contains("RelClipToWorld"))
    {
        FArchive local_Ar(content["RelClipToWorld"], Ar);
        TStaticSerializer<decltype(this->RelClipToWorld)>::Deserialize(this->RelClipToWorld, local_Ar);
    }
    if (content.contains("RelWorldToClip"))
    {
        FArchive local_Ar(content["RelWorldToClip"], Ar);
        TStaticSerializer<decltype(this->RelWorldToClip)>::Deserialize(this->RelWorldToClip, local_Ar);
    }
    if (content.contains("RelWorldToView"))
    {
        FArchive local_Ar(content["RelWorldToView"], Ar);
        TStaticSerializer<decltype(this->RelWorldToView)>::Deserialize(this->RelWorldToView, local_Ar);
    }
    if (content.contains("ViewToClip"))
    {
        FArchive local_Ar(content["ViewToClip"], Ar);
        TStaticSerializer<decltype(this->ViewToClip)>::Deserialize(this->ViewToClip, local_Ar);
    }
    
}
