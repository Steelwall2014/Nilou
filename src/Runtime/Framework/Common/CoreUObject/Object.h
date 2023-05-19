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
    
    class NCLASS NAsset : public NObject
    {
    public: 
        GENERATED_BODY()

        /**
         * @brief The path of the file which contains this object.
         * The path is used for serialization, relative to FPath::ContentDir().
         * Note: If it's empty, then the object will not be written to disk while serializing.
         * It will be automatically filled before deserializing.
         */
        std::filesystem::path SerializationPath;

        class FContentEntry* ContentEntry;
    };

    template<typename T>
    T* Cast(SharedObject Object)
    {
        auto obj = Object.StaticCast(Ubpa::Type_of<T>);
        if (obj)
            return obj.AsPtr<T>();
        return nullptr;
    }
    
}