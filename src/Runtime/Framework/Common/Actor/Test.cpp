#include "Test.h"
#include "Test2.h"
#include "Common/CoreUObject/Package.h"

namespace nilou {

FTestStructA FTestStructA::Construct()
{
    FTestStructA Struct;
    for (int i = 0; i < 4; i++)
    {
        NPackage* Package = CreatePackage("/Test/Serailization/FTestStructA/Array/Package_" + std::to_string(i));
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_" + std::to_string(i));
        Struct.BArray.Add(Object);
    }
    for (int i = 0; i < 5; i++)
    {
        NPackage* Package = CreatePackage("/Test/Serailization/FTestStructA/Map/Package_" + std::to_string(i));
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_" + std::to_string(i));
        Struct.BMap.Add(std::to_string(i), Object);
    }
    for (int i = 0; i < 6; i++)
    {
        NPackage* Package = CreatePackage("/Test/Serailization/FTestStructA/Set/Package_" + std::to_string(i));
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_" + std::to_string(i));
        Struct.BArray.Add(Object);
    }
    return Struct;
}

FTestStructB FTestStructB::Construct()
{
    FTestStructB Struct;
    Struct.A = FTestStructA::Construct();
    return Struct;
}

NTestObjectA* NTestObjectA::Construct()
{
    
}

}