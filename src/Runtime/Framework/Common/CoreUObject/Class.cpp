#include "Class.h"
#include "InheritanceGraph.h"

namespace nilou {
    
    bool UClass::IsChildOf(const UClass *BaseClass) const
    {
        return FInheritanceGraph::GetInheritanceGraph()->IsDerived(ClassEnum, BaseClass->ClassEnum);
    }

}