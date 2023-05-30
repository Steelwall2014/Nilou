#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FShadowMapFrustumIndex::StaticClass_ = nullptr;
const NClass *nilou::FShadowMapFrustumIndex::GetClass() const 
{ 
    return nilou::FShadowMapFrustumIndex::StaticClass(); 
}
const NClass *nilou::FShadowMapFrustumIndex::StaticClass()
{
    return nilou::FShadowMapFrustumIndex::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FShadowMapFrustumIndex>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FShadowMapFrustumIndex::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FShadowMapFrustumIndex>();
		Mngr.AddMethod<&nilou::FShadowMapFrustumIndex::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FShadowMapFrustumIndex::Serialize>("Serialize");
;
        nilou::FShadowMapFrustumIndex::StaticClass_->Type = Type_of<nilou::FShadowMapFrustumIndex>;
        nilou::FShadowMapFrustumIndex::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FShadowMapFrustumIndex>);
    }

    static TClassRegistry<nilou::FShadowMapFrustumIndex> Dummy;
};
TClassRegistry<nilou::FShadowMapFrustumIndex> Dummy = TClassRegistry<nilou::FShadowMapFrustumIndex>("nilou::FShadowMapFrustumIndex");



void nilou::FShadowMapFrustumIndex::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

}

void nilou::FShadowMapFrustumIndex::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    
}
