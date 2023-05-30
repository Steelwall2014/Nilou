#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FBuildMinMaxBlock::StaticClass_ = nullptr;
const NClass *nilou::FBuildMinMaxBlock::GetClass() const 
{ 
    return nilou::FBuildMinMaxBlock::StaticClass(); 
}
const NClass *nilou::FBuildMinMaxBlock::StaticClass()
{
    return nilou::FBuildMinMaxBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FBuildMinMaxBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FBuildMinMaxBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FBuildMinMaxBlock>();
		Mngr.AddField<&nilou::FBuildMinMaxBlock::Offset>("Offset");
		Mngr.AddMethod<&nilou::FBuildMinMaxBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FBuildMinMaxBlock::Serialize>("Serialize");
;
        nilou::FBuildMinMaxBlock::StaticClass_->Type = Type_of<nilou::FBuildMinMaxBlock>;
        nilou::FBuildMinMaxBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FBuildMinMaxBlock>);
    }

    static TClassRegistry<nilou::FBuildMinMaxBlock> Dummy;
};
TClassRegistry<nilou::FBuildMinMaxBlock> Dummy = TClassRegistry<nilou::FBuildMinMaxBlock>("nilou::FBuildMinMaxBlock");



void nilou::FBuildMinMaxBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Offset"], Ar);
        TStaticSerializer<decltype(this->Offset)>::Serialize(this->Offset, local_Ar);
    }
}

void nilou::FBuildMinMaxBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Offset"))
    {
        FArchive local_Ar(content["Offset"], Ar);
        TStaticSerializer<decltype(this->Offset)>::Deserialize(this->Offset, local_Ar);
    }
    
}
