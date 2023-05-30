#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FBuildNormalTangentBlock::StaticClass_ = nullptr;
const NClass *nilou::FBuildNormalTangentBlock::GetClass() const 
{ 
    return nilou::FBuildNormalTangentBlock::StaticClass(); 
}
const NClass *nilou::FBuildNormalTangentBlock::StaticClass()
{
    return nilou::FBuildNormalTangentBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FBuildNormalTangentBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FBuildNormalTangentBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FBuildNormalTangentBlock>();
		Mngr.AddField<&nilou::FBuildNormalTangentBlock::HeightfieldHeight>("HeightfieldHeight");
		Mngr.AddField<&nilou::FBuildNormalTangentBlock::HeightfieldWidth>("HeightfieldWidth");
		Mngr.AddField<&nilou::FBuildNormalTangentBlock::PixelMeterSize>("PixelMeterSize");
		Mngr.AddMethod<&nilou::FBuildNormalTangentBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FBuildNormalTangentBlock::Serialize>("Serialize");
;
        nilou::FBuildNormalTangentBlock::StaticClass_->Type = Type_of<nilou::FBuildNormalTangentBlock>;
        nilou::FBuildNormalTangentBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FBuildNormalTangentBlock>);
    }

    static TClassRegistry<nilou::FBuildNormalTangentBlock> Dummy;
};
TClassRegistry<nilou::FBuildNormalTangentBlock> Dummy = TClassRegistry<nilou::FBuildNormalTangentBlock>("nilou::FBuildNormalTangentBlock");



void nilou::FBuildNormalTangentBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["HeightfieldHeight"], Ar);
        TStaticSerializer<decltype(this->HeightfieldHeight)>::Serialize(this->HeightfieldHeight, local_Ar);
    }
    {
        FArchive local_Ar(content["HeightfieldWidth"], Ar);
        TStaticSerializer<decltype(this->HeightfieldWidth)>::Serialize(this->HeightfieldWidth, local_Ar);
    }
    {
        FArchive local_Ar(content["PixelMeterSize"], Ar);
        TStaticSerializer<decltype(this->PixelMeterSize)>::Serialize(this->PixelMeterSize, local_Ar);
    }
}

void nilou::FBuildNormalTangentBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("HeightfieldHeight"))
    {
        FArchive local_Ar(content["HeightfieldHeight"], Ar);
        TStaticSerializer<decltype(this->HeightfieldHeight)>::Deserialize(this->HeightfieldHeight, local_Ar);
    }
    if (content.contains("HeightfieldWidth"))
    {
        FArchive local_Ar(content["HeightfieldWidth"], Ar);
        TStaticSerializer<decltype(this->HeightfieldWidth)>::Deserialize(this->HeightfieldWidth, local_Ar);
    }
    if (content.contains("PixelMeterSize"))
    {
        FArchive local_Ar(content["PixelMeterSize"], Ar);
        TStaticSerializer<decltype(this->PixelMeterSize)>::Deserialize(this->PixelMeterSize, local_Ar);
    }
    
}
