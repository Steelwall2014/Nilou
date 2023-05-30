#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FQuadTreeParameters::StaticClass_ = nullptr;
const NClass *nilou::FQuadTreeParameters::GetClass() const 
{ 
    return nilou::FQuadTreeParameters::StaticClass(); 
}
const NClass *nilou::FQuadTreeParameters::StaticClass()
{
    return nilou::FQuadTreeParameters::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FQuadTreeParameters>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FQuadTreeParameters::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FQuadTreeParameters>();
		Mngr.AddField<&nilou::FQuadTreeParameters::LODNum>("LODNum");
		Mngr.AddField<&nilou::FQuadTreeParameters::NodeCount>("NodeCount");
		Mngr.AddField<&nilou::FQuadTreeParameters::NumHeightfieldTextureMipmap>("NumHeightfieldTextureMipmap");
		Mngr.AddField<&nilou::FQuadTreeParameters::NumPatchesPerNode>("NumPatchesPerNode");
		Mngr.AddField<&nilou::FQuadTreeParameters::NumQuadsPerPatch>("NumQuadsPerPatch");
		Mngr.AddMethod<&nilou::FQuadTreeParameters::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FQuadTreeParameters::Serialize>("Serialize");
;
        nilou::FQuadTreeParameters::StaticClass_->Type = Type_of<nilou::FQuadTreeParameters>;
        nilou::FQuadTreeParameters::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FQuadTreeParameters>);
    }

    static TClassRegistry<nilou::FQuadTreeParameters> Dummy;
};
TClassRegistry<nilou::FQuadTreeParameters> Dummy = TClassRegistry<nilou::FQuadTreeParameters>("nilou::FQuadTreeParameters");



void nilou::FQuadTreeParameters::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["LODNum"], Ar);
        TStaticSerializer<decltype(this->LODNum)>::Serialize(this->LODNum, local_Ar);
    }
    {
        FArchive local_Ar(content["NodeCount"], Ar);
        TStaticSerializer<decltype(this->NodeCount)>::Serialize(this->NodeCount, local_Ar);
    }
    {
        FArchive local_Ar(content["NumHeightfieldTextureMipmap"], Ar);
        TStaticSerializer<decltype(this->NumHeightfieldTextureMipmap)>::Serialize(this->NumHeightfieldTextureMipmap, local_Ar);
    }
    {
        FArchive local_Ar(content["NumPatchesPerNode"], Ar);
        TStaticSerializer<decltype(this->NumPatchesPerNode)>::Serialize(this->NumPatchesPerNode, local_Ar);
    }
    {
        FArchive local_Ar(content["NumQuadsPerPatch"], Ar);
        TStaticSerializer<decltype(this->NumQuadsPerPatch)>::Serialize(this->NumQuadsPerPatch, local_Ar);
    }
}

void nilou::FQuadTreeParameters::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("LODNum"))
    {
        FArchive local_Ar(content["LODNum"], Ar);
        TStaticSerializer<decltype(this->LODNum)>::Deserialize(this->LODNum, local_Ar);
    }
    if (content.contains("NodeCount"))
    {
        FArchive local_Ar(content["NodeCount"], Ar);
        TStaticSerializer<decltype(this->NodeCount)>::Deserialize(this->NodeCount, local_Ar);
    }
    if (content.contains("NumHeightfieldTextureMipmap"))
    {
        FArchive local_Ar(content["NumHeightfieldTextureMipmap"], Ar);
        TStaticSerializer<decltype(this->NumHeightfieldTextureMipmap)>::Deserialize(this->NumHeightfieldTextureMipmap, local_Ar);
    }
    if (content.contains("NumPatchesPerNode"))
    {
        FArchive local_Ar(content["NumPatchesPerNode"], Ar);
        TStaticSerializer<decltype(this->NumPatchesPerNode)>::Deserialize(this->NumPatchesPerNode, local_Ar);
    }
    if (content.contains("NumQuadsPerPatch"))
    {
        FArchive local_Ar(content["NumQuadsPerPatch"], Ar);
        TStaticSerializer<decltype(this->NumQuadsPerPatch)>::Deserialize(this->NumQuadsPerPatch, local_Ar);
    }
    
}
