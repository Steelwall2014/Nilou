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

        virtual void Serialize(nlohmann::json &json, const std::filesystem::path &Path) { }

        /**
         * @param json The out json object
         * @param Path The path of current object, relative to FPath::ContentDir(). 
         * It is used for referencing other contents.
         */
        virtual void Deserialize(nlohmann::json &json, const std::filesystem::path &Path) { }

    };

    class FObjectFactory
    {
    public:
        static std::unique_ptr<UObject> CreateDefaultObjectByName(const std::string &ClassName);
    private:
        FObjectFactory();
        std::unordered_map<std::string, std::function<std::unique_ptr<UObject>(void)>> FunctionMap;
    };
}