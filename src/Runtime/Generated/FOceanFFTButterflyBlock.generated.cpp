#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FOceanFFTButterflyBlock::StaticClass_ = nullptr;
const NClass *nilou::FOceanFFTButterflyBlock::GetClass() const 
{ 
    return nilou::FOceanFFTButterflyBlock::StaticClass(); 
}
const NClass *nilou::FOceanFFTButterflyBlock::StaticClass()
{
    return nilou::FOceanFFTButterflyBlock::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FOceanFFTButterflyBlock>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FOceanFFTButterflyBlock::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FOceanFFTButterflyBlock>();
		Mngr.AddField<&nilou::FOceanFFTButterflyBlock::Ns>("Ns");
		Mngr.AddMethod<&nilou::FOceanFFTButterflyBlock::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FOceanFFTButterflyBlock::Serialize>("Serialize");
;
        nilou::FOceanFFTButterflyBlock::StaticClass_->Type = Type_of<nilou::FOceanFFTButterflyBlock>;
        nilou::FOceanFFTButterflyBlock::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FOceanFFTButterflyBlock>);
    }

    static TClassRegistry<nilou::FOceanFFTButterflyBlock> Dummy;
};
TClassRegistry<nilou::FOceanFFTButterflyBlock> Dummy = TClassRegistry<nilou::FOceanFFTButterflyBlock>("nilou::FOceanFFTButterflyBlock");



void nilou::FOceanFFTButterflyBlock::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Ns"], Ar);
        TStaticSerializer<decltype(this->Ns)>::Serialize(this->Ns, local_Ar);
    }
}

void nilou::FOceanFFTButterflyBlock::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Ns"))
    {
        FArchive local_Ar(content["Ns"], Ar);
        TStaticSerializer<decltype(this->Ns)>::Deserialize(this->Ns, local_Ar);
    }
    
}
