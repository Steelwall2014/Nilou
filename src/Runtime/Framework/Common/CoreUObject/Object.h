#pragma once
#include "Class.h"
#include "MarkedClasses.generated.h"
#include "Templates/ObjectMacros.h"
// #include "Object.generated.h"
namespace nilou {
    class UClass;
    
    UCLASS()
    class UObject 
    {
    public: 
        GENERATE_CLASS_INFO()
        
        bool IsA(const UClass *Class);

    };
}