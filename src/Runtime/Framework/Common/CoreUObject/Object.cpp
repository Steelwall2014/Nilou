#include "Object.h"
#include "Common/CoreUObject/Class.h"
#include "InheritanceGraph.h"

namespace nilou {

    bool UObject::IsA(const UClass *Class)
    {
        return FInheritanceGraph::GetInheritanceGraph()->IsDerived(GetClassEnum(), Class->ClassEnum);
    }


    std::unique_ptr<UObject> FObjectFactory::CreateDefaultObjectByName(const std::string &ClassName)
    {
        static FObjectFactory DummyObject;
        if (DummyObject.FunctionMap.find(ClassName) == DummyObject.FunctionMap.end())
            return nullptr;
        return DummyObject.FunctionMap[ClassName]();
    }
}