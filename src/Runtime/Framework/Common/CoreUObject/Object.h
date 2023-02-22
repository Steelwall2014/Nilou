#pragma once
#include <json/json.hpp>
#include "Class.h"
#include "MarkedClasses.generated.h"
#include "Templates/ObjectMacros.h"
#include "SerializeHelper.h"
// #include "Object.generated.h"
namespace nilou {
    class UClass;
    
    UCLASS()
    class UObject 
    {
    public: 
        GENERATE_CLASS_INFO()
        
        bool IsA(const UClass *Class);

        virtual void Serialize(nlohmann::json &json) { }

        virtual void Deserialize(nlohmann::json &json) { }

    };
}