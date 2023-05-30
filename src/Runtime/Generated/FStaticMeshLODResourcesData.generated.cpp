#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FStaticMeshLODResourcesData::StaticClass_ = nullptr;
const NClass *nilou::FStaticMeshLODResourcesData::GetClass() const 
{ 
    return nilou::FStaticMeshLODResourcesData::StaticClass(); 
}
const NClass *nilou::FStaticMeshLODResourcesData::StaticClass()
{
    return nilou::FStaticMeshLODResourcesData::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FStaticMeshLODResourcesData>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FStaticMeshLODResourcesData::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FStaticMeshLODResourcesData>();
		Mngr.AddField<&nilou::FStaticMeshLODResourcesData::Sections>("Sections");
		Mngr.AddMethod<&nilou::FStaticMeshLODResourcesData::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FStaticMeshLODResourcesData::Serialize>("Serialize");
;
        nilou::FStaticMeshLODResourcesData::StaticClass_->Type = Type_of<nilou::FStaticMeshLODResourcesData>;
        nilou::FStaticMeshLODResourcesData::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FStaticMeshLODResourcesData>);
    }

    static TClassRegistry<nilou::FStaticMeshLODResourcesData> Dummy;
};
TClassRegistry<nilou::FStaticMeshLODResourcesData> Dummy = TClassRegistry<nilou::FStaticMeshLODResourcesData>("nilou::FStaticMeshLODResourcesData");



void nilou::FStaticMeshLODResourcesData::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Sections"], Ar);
        TStaticSerializer<decltype(this->Sections)>::Serialize(this->Sections, local_Ar);
    }
}

void nilou::FStaticMeshLODResourcesData::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Sections"))
    {
        FArchive local_Ar(content["Sections"], Ar);
        TStaticSerializer<decltype(this->Sections)>::Deserialize(this->Sections, local_Ar);
    }
    
}
