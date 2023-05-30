#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FCreatePatchBlock::StaticClass_ = nullptr;
const NClass *nilou::FCreatePatchBlock::GetClass() const 
{ 
    return nilou::FCreatePatchBlock::StaticClass(); 
}
const NClass *nilou::FCreatePatchBlock::StaticClass()
{
    return nilou::FCreatePatchBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FCreatePatchBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FCreatePatchBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FCreatePatchBlock>();
		Mngr.AddField<&nilou::FCreatePatchBlock::LodTextureSize>("LodTextureSize");
		Mngr.AddMethod<&nilou::FCreatePatchBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FCreatePatchBlock::Serialize>("Serialize");
;
        nilou::FCreatePatchBlock::StaticClass_->Type = Type_of<nilou::FCreatePatchBlock>;
        nilou::FCreatePatchBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FCreatePatchBlock>);
    }

    static TClassRegistry<nilou::FCreatePatchBlock> Dummy;
};
TClassRegistry<nilou::FCreatePatchBlock> Dummy = TClassRegistry<nilou::FCreatePatchBlock>("nilou::FCreatePatchBlock");



void nilou::FCreatePatchBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["LodTextureSize"], Ar);
        TStaticSerializer<decltype(this->LodTextureSize)>::Serialize(this->LodTextureSize, local_Ar);
    }
}

void nilou::FCreatePatchBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("LodTextureSize"))
    {
        FArchive local_Ar(content["LodTextureSize"], Ar);
        TStaticSerializer<decltype(this->LodTextureSize)>::Deserialize(this->LodTextureSize, local_Ar);
    }
    
}
