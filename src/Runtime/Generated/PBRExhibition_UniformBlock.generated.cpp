#include "D:/Nilou/src/Runtime/Rendering/MaterialUniformBlocks.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::PBRExhibition_UniformBlock::StaticClass_ = nullptr;
const NClass *nilou::PBRExhibition_UniformBlock::GetClass() const 
{ 
    return nilou::PBRExhibition_UniformBlock::StaticClass(); 
}
const NClass *nilou::PBRExhibition_UniformBlock::StaticClass()
{
    return nilou::PBRExhibition_UniformBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::PBRExhibition_UniformBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::PBRExhibition_UniformBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::PBRExhibition_UniformBlock>();
		Mngr.AddField<&nilou::PBRExhibition_UniformBlock::Blue>("Blue");
		Mngr.AddField<&nilou::PBRExhibition_UniformBlock::Green>("Green");
		Mngr.AddField<&nilou::PBRExhibition_UniformBlock::Metallic>("Metallic");
		Mngr.AddField<&nilou::PBRExhibition_UniformBlock::Red>("Red");
		Mngr.AddField<&nilou::PBRExhibition_UniformBlock::Roughness>("Roughness");
		Mngr.AddMethod<&nilou::PBRExhibition_UniformBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::PBRExhibition_UniformBlock::Serialize>("Serialize");
;
        nilou::PBRExhibition_UniformBlock::StaticClass_->Type = Type_of<nilou::PBRExhibition_UniformBlock>;
        nilou::PBRExhibition_UniformBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::PBRExhibition_UniformBlock>);
    }

    static TClassRegistry<nilou::PBRExhibition_UniformBlock> Dummy;
};
TClassRegistry<nilou::PBRExhibition_UniformBlock> Dummy = TClassRegistry<nilou::PBRExhibition_UniformBlock>("nilou::PBRExhibition_UniformBlock");



void nilou::PBRExhibition_UniformBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Blue"], Ar);
        TStaticSerializer<decltype(this->Blue)>::Serialize(this->Blue, local_Ar);
    }
    {
        FArchive local_Ar(content["Green"], Ar);
        TStaticSerializer<decltype(this->Green)>::Serialize(this->Green, local_Ar);
    }
    {
        FArchive local_Ar(content["Metallic"], Ar);
        TStaticSerializer<decltype(this->Metallic)>::Serialize(this->Metallic, local_Ar);
    }
    {
        FArchive local_Ar(content["Red"], Ar);
        TStaticSerializer<decltype(this->Red)>::Serialize(this->Red, local_Ar);
    }
    {
        FArchive local_Ar(content["Roughness"], Ar);
        TStaticSerializer<decltype(this->Roughness)>::Serialize(this->Roughness, local_Ar);
    }
}

void nilou::PBRExhibition_UniformBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Blue"))
    {
        FArchive local_Ar(content["Blue"], Ar);
        TStaticSerializer<decltype(this->Blue)>::Deserialize(this->Blue, local_Ar);
    }
    if (content.contains("Green"))
    {
        FArchive local_Ar(content["Green"], Ar);
        TStaticSerializer<decltype(this->Green)>::Deserialize(this->Green, local_Ar);
    }
    if (content.contains("Metallic"))
    {
        FArchive local_Ar(content["Metallic"], Ar);
        TStaticSerializer<decltype(this->Metallic)>::Deserialize(this->Metallic, local_Ar);
    }
    if (content.contains("Red"))
    {
        FArchive local_Ar(content["Red"], Ar);
        TStaticSerializer<decltype(this->Red)>::Deserialize(this->Red, local_Ar);
    }
    if (content.contains("Roughness"))
    {
        FArchive local_Ar(content["Roughness"], Ar);
        TStaticSerializer<decltype(this->Roughness)>::Deserialize(this->Roughness, local_Ar);
    }
    
}
