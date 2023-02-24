#pragma once
#include <json/json.hpp>
#include "Class.h"
#include "MarkedClasses.generated.h"
#include "Templates/ObjectMacros.h"
#include "SerializeHelper.h"
#include "Archive.h"
// #include "Object.generated.h"
namespace nilou {
    class UClass;
    
    UCLASS()
    class UObject 
    {
    public: 
        GENERATE_CLASS_INFO()
        

        /**
         * @brief Returns whether the class of the current object is derived from given Class parameter
         */
        bool IsA(const UClass *Class);

        virtual void Serialize(FArchive &Ar) { }

        virtual void Deserialize(FArchive &Ar) { }

        /**
         * @brief The path of the file which contains this object.
         * The path is used for serialization, relative to FPath::ContentDir().
         * Note: If it's empty, then the object will not be written to disk while serializing.
         * It will be automatically filled while deserializing.
         */
        std::filesystem::path SerializationPath;
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