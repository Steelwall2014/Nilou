#pragma once
#include <json/json.hpp>
#include <reflection/Class.h>
#include "Templates/ObjectMacros.h"
#include "SerializeHelper.h"
#include "Archive.h"
// #include "Object.generated.h"
namespace nilou {
    using NClass = reflection::NClass;
    
    class NCLASS UObject 
    {
    public: 
        GENERATE_BODY()
        

        /**
         * @brief Returns whether the class of the current object is derived from given Class parameter
         */
        bool IsA(const NClass *Class);

        const std::string& GetClassName() const;

        virtual void Serialize(FArchive &Ar) { }

        virtual void Deserialize(FArchive &Ar) { }

        /**
         * @brief The path of the file which contains this object.
         * The path is used for serialization, relative to FPath::ContentDir().
         * Note: If it's empty, then the object will not be written to disk while serializing.
         * It will be automatically filled before deserializing.
         */
        std::filesystem::path SerializationPath;

        class FContentEntry* ContentEntry;
    };

    void* CreateDefaultObjectByName(const std::string &ClassName);
    
}