#include "D:/Nilou/src/Runtime/Rendering/SceneView.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FBoundingBox::StaticClass_ = nullptr;
const NClass *nilou::FBoundingBox::GetClass() const 
{ 
    return nilou::FBoundingBox::StaticClass(); 
}
const NClass *nilou::FBoundingBox::StaticClass()
{
    return nilou::FBoundingBox::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FBoundingBox>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FBoundingBox::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FBoundingBox>();
		Mngr.AddField<&nilou::FBoundingBox::Max>("Max");
		Mngr.AddField<&nilou::FBoundingBox::Min>("Min");
;
        nilou::FBoundingBox::StaticClass_->Type = Type_of<nilou::FBoundingBox>;
        nilou::FBoundingBox::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FBoundingBox>);
    }

    static TClassRegistry<nilou::FBoundingBox> Dummy;
};
TClassRegistry<nilou::FBoundingBox> Dummy = TClassRegistry<nilou::FBoundingBox>("nilou::FBoundingBox");



void nilou::FBoundingBox::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Max"], Ar);
        TStaticSerializer<decltype(this->Max)>::Serialize(this->Max, local_Ar);
    }
    {
        FArchive local_Ar(content["Min"], Ar);
        TStaticSerializer<decltype(this->Min)>::Serialize(this->Min, local_Ar);
    }
}

void nilou::FBoundingBox::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Max"))
    {
        FArchive local_Ar(content["Max"], Ar);
        TStaticSerializer<decltype(this->Max)>::Deserialize(this->Max, local_Ar);
    }
    if (content.contains("Min"))
    {
        FArchive local_Ar(content["Min"], Ar);
        TStaticSerializer<decltype(this->Min)>::Deserialize(this->Min, local_Ar);
    }
    
}
