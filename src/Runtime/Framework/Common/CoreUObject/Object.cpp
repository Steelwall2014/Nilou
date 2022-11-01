#include "Object.h"
#include "Common/CoreUObject/Class.h"
#include "Common/InheritanceGraph.h"

namespace nilou {

    bool UObject::IsA(const UClass *Class)
    {
        return FInheritanceGraph::GetInheritanceGraph()->IsDerived(GetClassEnum(), Class->ClassEnum);
    }
}