#pragma once
#include <json/json.hpp>
#include <reflection/Class.h>
#include "Templates/ObjectMacros.h"
#include "SerializeHelper.h"
#include "Archive.h"
// #include "Object.generated.h"
namespace nilou {

    using ObjectView = Ubpa::UDRefl::ObjectView;
    using SharedObject = Ubpa::UDRefl::SharedObject;
    
    class NCLASS UObject 
    {
    public: 
        GENERATED_BODY()
        

        /**
         * @brief Returns whether the class of the current object is derived from given Class parameter
         */
        NFUNCTION()
        bool IsA(const NClass *Class);

        NFUNCTION()
        std::string_view GetClassName() const;

        NFUNCTION()
        virtual void Serialize(FArchive &Ar) { }

        NFUNCTION()
        virtual void Deserialize(FArchive &Ar) { }

        /**
         * @brief The path of the file which contains this object.
         * The path is used for serialization, relative to FPath::ContentDir().
         * Note: If it's empty, then the object will not be written to disk while serializing.
         * It will be automatically filled before deserializing.
         */
        NPROPERTY()
        std::filesystem::path SerializationPath;

        NPROPERTY()
        class FContentEntry* ContentEntry;
    };

    UObject* CreateDefaultObjectByName(const std::string &ClassName);

    template<typename T>
    T* Cast(SharedObject Object)
    {
        auto obj = Object.StaticCast(Ubpa::Type_of<T>);
        if (obj)
            return obj.AsPtr<T>();
        return nullptr;
    }
    
}