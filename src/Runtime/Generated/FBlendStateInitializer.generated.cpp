#include "D:/Nilou/src/Runtime/RHI/RHI.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FBlendStateInitializer::StaticClass_ = nullptr;
const NClass *nilou::FBlendStateInitializer::GetClass() const 
{ 
    return nilou::FBlendStateInitializer::StaticClass(); 
}
const NClass *nilou::FBlendStateInitializer::StaticClass()
{
    return nilou::FBlendStateInitializer::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FBlendStateInitializer>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FBlendStateInitializer::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FBlendStateInitializer>();
		Mngr.AddField<&nilou::FBlendStateInitializer::RenderTargets>("RenderTargets");
		Mngr.AddField<&nilou::FBlendStateInitializer::bUseIndependentRenderTargetBlendStates>("bUseIndependentRenderTargetBlendStates");
;
        nilou::FBlendStateInitializer::StaticClass_->Type = Type_of<nilou::FBlendStateInitializer>;
        nilou::FBlendStateInitializer::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FBlendStateInitializer>);
    }

    static TClassRegistry<nilou::FBlendStateInitializer> Dummy;
};
TClassRegistry<nilou::FBlendStateInitializer> Dummy = TClassRegistry<nilou::FBlendStateInitializer>("nilou::FBlendStateInitializer");



void nilou::FBlendStateInitializer::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["RenderTargets"], Ar);
        TStaticSerializer<decltype(this->RenderTargets)>::Serialize(this->RenderTargets, local_Ar);
    }
    {
        FArchive local_Ar(content["bUseIndependentRenderTargetBlendStates"], Ar);
        TStaticSerializer<decltype(this->bUseIndependentRenderTargetBlendStates)>::Serialize(this->bUseIndependentRenderTargetBlendStates, local_Ar);
    }
}

void nilou::FBlendStateInitializer::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("RenderTargets"))
    {
        FArchive local_Ar(content["RenderTargets"], Ar);
        TStaticSerializer<decltype(this->RenderTargets)>::Deserialize(this->RenderTargets, local_Ar);
    }
    if (content.contains("bUseIndependentRenderTargetBlendStates"))
    {
        FArchive local_Ar(content["bUseIndependentRenderTargetBlendStates"], Ar);
        TStaticSerializer<decltype(this->bUseIndependentRenderTargetBlendStates)>::Deserialize(this->bUseIndependentRenderTargetBlendStates, local_Ar);
    }
    
}
