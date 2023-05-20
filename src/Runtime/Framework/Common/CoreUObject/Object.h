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
    
    class FContentManager *GetContentManager();
}

template<typename  T>
class TStaticSerializer<std::shared_ptr<T>>
{
    using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
public:
    static void Serialize(std::shared_ptr<T>& Object, FArchive& Ar) 
    { 
        if (Object)
        {
            if (T::StaticClass()->IsChildOf(nilou::NAsset::StaticClass()))
            {
                auto asset = std::static_pointer_cast<nilou::NAsset>(Object);
                Ar.Node = asset->SerializationPath.generic_string();
            }
            else 
            {
                Object->Serialize(Ar);
            }
        }
    }
    static void Deserialize(std::shared_ptr<T>& Object, FArchive& Ar) 
    { 
        if (T::StaticClass()->IsChildOf(nilou::NAsset::StaticClass()) && Ar.Node.is_string())
        {
            std::filesystem::path path = std::filesystem::path(Ar.Node.get<std::string>());
            Object = std::static_pointer_cast<T>(nilou::GetContentManager()->GetContentByPath(path));
        }
        else if (Ar.Node.contains("ClassName"))
        {
            std::string class_name = Ar.Node["ClassName"];
            if (Object == nullptr)
                Object = std::shared_ptr<T>(static_cast<T*>(CreateDefaultObject(class_name)));

            if (Object)
                Object->Deserialize(Ar);
        }
    }
};

template<typename T>
class TStaticSerializer<T*>
{
    using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
public:
    static void Serialize(T*& Object, FArchive& Ar) 
    { 
        if (Object)
        {
            if (nilou::TIsDerivedFrom<T, nilou::NAsset>::Value)
            {
                auto asset = static_cast<nilou::NAsset*>(Object);
                Ar.Node = asset->SerializationPath.generic_string();
            }
            else 
            {
                Object->Serialize(Ar);
            }
        }
    }
    static void Deserialize(T*& Object, FArchive& Ar) 
    { 
        if (nilou::TIsDerivedFrom<T, nilou::NAsset>::Value && Ar.Node.is_string())
        {
            std::filesystem::path path = std::filesystem::path(Ar.Node.get<std::string>());
            Object = static_cast<T*>(nilou::GetContentManager()->GetContentByPath(path));
        }
        else if (Ar.Node.contains("ClassName"))
        {
            std::string class_name = Ar.Node["ClassName"];
            if (Object == nullptr)
                Object = static_cast<T*>(CreateDefaultObject(class_name));

            if (Object)
                Object->Deserialize(Ar);
        }
    }
};