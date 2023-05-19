#include "D:/Nilou/src/Runtime/Framework/Common/Components/VirtualHeightfieldMeshComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FStaticMeshSectionData::StaticClass_ = nullptr;
const NClass *nilou::FStaticMeshSectionData::GetClass() const 
{ 
    return nilou::FStaticMeshSectionData::StaticClass(); 
}
const NClass *nilou::FStaticMeshSectionData::StaticClass()
{
    return nilou::FStaticMeshSectionData::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FStaticMeshSectionData>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FStaticMeshSectionData::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FStaticMeshSectionData>();
		Mngr.AddField<&nilou::FStaticMeshSectionData::Colors>("Colors");
		Mngr.AddField<&nilou::FStaticMeshSectionData::IndexBuffer>("IndexBuffer");
		Mngr.AddField<&nilou::FStaticMeshSectionData::MaterialIndex>("MaterialIndex");
		Mngr.AddField<&nilou::FStaticMeshSectionData::Normals>("Normals");
		Mngr.AddField<&nilou::FStaticMeshSectionData::Positions>("Positions");
		Mngr.AddField<&nilou::FStaticMeshSectionData::Tangents>("Tangents");
		Mngr.AddField<&nilou::FStaticMeshSectionData::TexCoords>("TexCoords");
		Mngr.AddField<&nilou::FStaticMeshSectionData::bCastShadow>("bCastShadow");
;
        nilou::FStaticMeshSectionData::StaticClass_->Type = Type_of<nilou::FStaticMeshSectionData>;
        nilou::FStaticMeshSectionData::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FStaticMeshSectionData>);
    }

    static TClassRegistry<nilou::FStaticMeshSectionData> Dummy;
};
TClassRegistry<nilou::FStaticMeshSectionData> Dummy = TClassRegistry<nilou::FStaticMeshSectionData>("nilou::FStaticMeshSectionData");



void nilou::FStaticMeshSectionData::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Colors"], Ar);
        TStaticSerializer<decltype(this->Colors)>::Serialize(this->Colors, local_Ar);
    }
    {
        FArchive local_Ar(content["IndexBuffer"], Ar);
        TStaticSerializer<decltype(this->IndexBuffer)>::Serialize(this->IndexBuffer, local_Ar);
    }
    {
        FArchive local_Ar(content["MaterialIndex"], Ar);
        TStaticSerializer<decltype(this->MaterialIndex)>::Serialize(this->MaterialIndex, local_Ar);
    }
    {
        FArchive local_Ar(content["Normals"], Ar);
        TStaticSerializer<decltype(this->Normals)>::Serialize(this->Normals, local_Ar);
    }
    {
        FArchive local_Ar(content["Positions"], Ar);
        TStaticSerializer<decltype(this->Positions)>::Serialize(this->Positions, local_Ar);
    }
    {
        FArchive local_Ar(content["Tangents"], Ar);
        TStaticSerializer<decltype(this->Tangents)>::Serialize(this->Tangents, local_Ar);
    }
    {
        FArchive local_Ar(content["TexCoords"], Ar);
        TStaticSerializer<decltype(this->TexCoords)>::Serialize(this->TexCoords, local_Ar);
    }
    {
        FArchive local_Ar(content["bCastShadow"], Ar);
        TStaticSerializer<decltype(this->bCastShadow)>::Serialize(this->bCastShadow, local_Ar);
    }
}

void nilou::FStaticMeshSectionData::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Colors"))
    {
        FArchive local_Ar(content["Colors"], Ar);
        TStaticSerializer<decltype(this->Colors)>::Deserialize(this->Colors, local_Ar);
    }
    if (content.contains("IndexBuffer"))
    {
        FArchive local_Ar(content["IndexBuffer"], Ar);
        TStaticSerializer<decltype(this->IndexBuffer)>::Deserialize(this->IndexBuffer, local_Ar);
    }
    if (content.contains("MaterialIndex"))
    {
        FArchive local_Ar(content["MaterialIndex"], Ar);
        TStaticSerializer<decltype(this->MaterialIndex)>::Deserialize(this->MaterialIndex, local_Ar);
    }
    if (content.contains("Normals"))
    {
        FArchive local_Ar(content["Normals"], Ar);
        TStaticSerializer<decltype(this->Normals)>::Deserialize(this->Normals, local_Ar);
    }
    if (content.contains("Positions"))
    {
        FArchive local_Ar(content["Positions"], Ar);
        TStaticSerializer<decltype(this->Positions)>::Deserialize(this->Positions, local_Ar);
    }
    if (content.contains("Tangents"))
    {
        FArchive local_Ar(content["Tangents"], Ar);
        TStaticSerializer<decltype(this->Tangents)>::Deserialize(this->Tangents, local_Ar);
    }
    if (content.contains("TexCoords"))
    {
        FArchive local_Ar(content["TexCoords"], Ar);
        TStaticSerializer<decltype(this->TexCoords)>::Deserialize(this->TexCoords, local_Ar);
    }
    if (content.contains("bCastShadow"))
    {
        FArchive local_Ar(content["bCastShadow"], Ar);
        TStaticSerializer<decltype(this->bCastShadow)>::Deserialize(this->bCastShadow, local_Ar);
    }
    
}
