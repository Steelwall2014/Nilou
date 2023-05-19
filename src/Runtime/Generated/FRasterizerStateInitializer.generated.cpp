#include "D:/Nilou/src/Runtime/RHI/RHI.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FRasterizerStateInitializer::StaticClass_ = nullptr;
const NClass *nilou::FRasterizerStateInitializer::GetClass() const 
{ 
    return nilou::FRasterizerStateInitializer::StaticClass(); 
}
const NClass *nilou::FRasterizerStateInitializer::StaticClass()
{
    return nilou::FRasterizerStateInitializer::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FRasterizerStateInitializer>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FRasterizerStateInitializer::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FRasterizerStateInitializer>();
		Mngr.AddField<&nilou::FRasterizerStateInitializer::CullMode>("CullMode");
		Mngr.AddField<&nilou::FRasterizerStateInitializer::FillMode>("FillMode");
;
        nilou::FRasterizerStateInitializer::StaticClass_->Type = Type_of<nilou::FRasterizerStateInitializer>;
        nilou::FRasterizerStateInitializer::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FRasterizerStateInitializer>);
    }

    static TClassRegistry<nilou::FRasterizerStateInitializer> Dummy;
};
TClassRegistry<nilou::FRasterizerStateInitializer> Dummy = TClassRegistry<nilou::FRasterizerStateInitializer>("nilou::FRasterizerStateInitializer");



void nilou::FRasterizerStateInitializer::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["CullMode"], Ar);
        TStaticSerializer<decltype(this->CullMode)>::Serialize(this->CullMode, local_Ar);
    }
    {
        FArchive local_Ar(content["FillMode"], Ar);
        TStaticSerializer<decltype(this->FillMode)>::Serialize(this->FillMode, local_Ar);
    }
}

void nilou::FRasterizerStateInitializer::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("CullMode"))
    {
        FArchive local_Ar(content["CullMode"], Ar);
        TStaticSerializer<decltype(this->CullMode)>::Deserialize(this->CullMode, local_Ar);
    }
    if (content.contains("FillMode"))
    {
        FArchive local_Ar(content["FillMode"], Ar);
        TStaticSerializer<decltype(this->FillMode)>::Deserialize(this->FillMode, local_Ar);
    }
    
}
