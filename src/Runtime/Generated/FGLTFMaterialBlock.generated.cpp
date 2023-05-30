#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FGLTFMaterialBlock::StaticClass_ = nullptr;
const NClass *nilou::FGLTFMaterialBlock::GetClass() const 
{ 
    return nilou::FGLTFMaterialBlock::StaticClass(); 
}
const NClass *nilou::FGLTFMaterialBlock::StaticClass()
{
    return nilou::FGLTFMaterialBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FGLTFMaterialBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FGLTFMaterialBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FGLTFMaterialBlock>();
		Mngr.AddField<&nilou::FGLTFMaterialBlock::baseColorFactor>("baseColorFactor");
		Mngr.AddField<&nilou::FGLTFMaterialBlock::emissiveFactor>("emissiveFactor");
		Mngr.AddField<&nilou::FGLTFMaterialBlock::metallicFactor>("metallicFactor");
		Mngr.AddField<&nilou::FGLTFMaterialBlock::roughnessFactor>("roughnessFactor");
		Mngr.AddMethod<&nilou::FGLTFMaterialBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FGLTFMaterialBlock::Serialize>("Serialize");
;
        nilou::FGLTFMaterialBlock::StaticClass_->Type = Type_of<nilou::FGLTFMaterialBlock>;
        nilou::FGLTFMaterialBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FGLTFMaterialBlock>);
    }

    static TClassRegistry<nilou::FGLTFMaterialBlock> Dummy;
};
TClassRegistry<nilou::FGLTFMaterialBlock> Dummy = TClassRegistry<nilou::FGLTFMaterialBlock>("nilou::FGLTFMaterialBlock");



void nilou::FGLTFMaterialBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["baseColorFactor"], Ar);
        TStaticSerializer<decltype(this->baseColorFactor)>::Serialize(this->baseColorFactor, local_Ar);
    }
    {
        FArchive local_Ar(content["emissiveFactor"], Ar);
        TStaticSerializer<decltype(this->emissiveFactor)>::Serialize(this->emissiveFactor, local_Ar);
    }
    {
        FArchive local_Ar(content["metallicFactor"], Ar);
        TStaticSerializer<decltype(this->metallicFactor)>::Serialize(this->metallicFactor, local_Ar);
    }
    {
        FArchive local_Ar(content["roughnessFactor"], Ar);
        TStaticSerializer<decltype(this->roughnessFactor)>::Serialize(this->roughnessFactor, local_Ar);
    }
}

void nilou::FGLTFMaterialBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("baseColorFactor"))
    {
        FArchive local_Ar(content["baseColorFactor"], Ar);
        TStaticSerializer<decltype(this->baseColorFactor)>::Deserialize(this->baseColorFactor, local_Ar);
    }
    if (content.contains("emissiveFactor"))
    {
        FArchive local_Ar(content["emissiveFactor"], Ar);
        TStaticSerializer<decltype(this->emissiveFactor)>::Deserialize(this->emissiveFactor, local_Ar);
    }
    if (content.contains("metallicFactor"))
    {
        FArchive local_Ar(content["metallicFactor"], Ar);
        TStaticSerializer<decltype(this->metallicFactor)>::Deserialize(this->metallicFactor, local_Ar);
    }
    if (content.contains("roughnessFactor"))
    {
        FArchive local_Ar(content["roughnessFactor"], Ar);
        TStaticSerializer<decltype(this->roughnessFactor)>::Deserialize(this->roughnessFactor, local_Ar);
    }
    
}
