#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UStaticMesh::StaticClass_ = nullptr;
const NClass *nilou::UStaticMesh::GetClass() const 
{ 
    return nilou::UStaticMesh::StaticClass(); 
}
const NClass *nilou::UStaticMesh::StaticClass()
{
    return nilou::UStaticMesh::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UStaticMesh>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UStaticMesh::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UStaticMesh>();
		Mngr.AddField<&nilou::UStaticMesh::LODResourcesData>("LODResourcesData");
		Mngr.AddField<&nilou::UStaticMesh::LocalBoundingBox>("LocalBoundingBox");
		Mngr.AddField<&nilou::UStaticMesh::MaterialSlots>("MaterialSlots");
		Mngr.AddField<&nilou::UStaticMesh::Name>("Name");
		Mngr.AddBases<nilou::UStaticMesh, nilou::NAsset>();
;
        nilou::UStaticMesh::StaticClass_->Type = Type_of<nilou::UStaticMesh>;
        nilou::UStaticMesh::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UStaticMesh>);
    }

    static TClassRegistry<nilou::UStaticMesh> Dummy;
};
TClassRegistry<nilou::UStaticMesh> Dummy = TClassRegistry<nilou::UStaticMesh>("nilou::UStaticMesh");



void nilou::UStaticMesh::Serialize(FArchive& Ar)
{
    nilou::NAsset::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UStaticMesh";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["LODResourcesData"], Ar);
        TStaticSerializer<decltype(this->LODResourcesData)>::Serialize(this->LODResourcesData, local_Ar);
    }
    {
        FArchive local_Ar(content["LocalBoundingBox"], Ar);
        this->LocalBoundingBox.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["MaterialSlots"], Ar);
        TStaticSerializer<decltype(this->MaterialSlots)>::Serialize(this->MaterialSlots, local_Ar);
    }
    {
        FArchive local_Ar(content["Name"], Ar);
        TStaticSerializer<decltype(this->Name)>::Serialize(this->Name, local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::UStaticMesh::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("LODResourcesData"))
    {
        FArchive local_Ar(content["LODResourcesData"], Ar);
        TStaticSerializer<decltype(this->LODResourcesData)>::Deserialize(this->LODResourcesData, local_Ar);
    }
    if (content.contains("LocalBoundingBox"))
    {
        FArchive local_Ar(content["LocalBoundingBox"], Ar);
        this->LocalBoundingBox.Deserialize(local_Ar);
    }
    if (content.contains("MaterialSlots"))
    {
        FArchive local_Ar(content["MaterialSlots"], Ar);
        TStaticSerializer<decltype(this->MaterialSlots)>::Deserialize(this->MaterialSlots, local_Ar);
    }
    if (content.contains("Name"))
    {
        FArchive local_Ar(content["Name"], Ar);
        TStaticSerializer<decltype(this->Name)>::Deserialize(this->Name, local_Ar);
    }
    nilou::NAsset::Deserialize(Ar);
}
