#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> ACesium3DTileset::StaticClass_ = nullptr;
const NClass *ACesium3DTileset::GetClass() const 
{ 
    return ACesium3DTileset::StaticClass(); 
}
const NClass *ACesium3DTileset::StaticClass()
{
    return ACesium3DTileset::StaticClass_.get();
}

template<>
struct TClassRegistry<ACesium3DTileset>
{
    TClassRegistry(const std::string& InName)
    {
        ACesium3DTileset::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<ACesium3DTileset>("ACesium3DTileset")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        ACesium3DTileset::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<ACesium3DTileset> Dummy;
};
TClassRegistry<ACesium3DTileset> Dummy = TClassRegistry<ACesium3DTileset>("ACesium3DTileset");


