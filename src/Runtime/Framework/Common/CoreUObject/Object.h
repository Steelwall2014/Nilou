#pragma once
#include <json/json.hpp>
#include "Class.h"
#include "Templates/ObjectMacros.h"
#include "SerializeHelper.h"
#include "Archive.h"
#include "Common/ContentManager.h"
// #include "Object.generated.h"
namespace nilou {

    using ObjectView = Ubpa::UDRefl::ObjectView;
    using SharedObject = Ubpa::UDRefl::SharedObject;
    
    class NCLASS NAsset : public NObject
    {
    public: 
        GENERATED_BODY()

        FContentEntry* ContentEntry;

        NPROPERTY()
        std::string Name;

        std::string GetVirtualPath() const { return ContentEntry->VirtualPath; }

        std::filesystem::path GetAbsolutePath() const { return ContentEntry->AbsolutePath; }

        void MarkAssetDirty() { ContentEntry->bIsDirty = true; }
    };

    template<typename T>
    T* Cast(SharedObject Object)
    {
        auto obj = Object.StaticCast(Ubpa::Type_of<T>);
        if (obj)
            return obj.template AsPtr<T>();
        return nullptr;
    }

    template <class T>
    T* Cast(NObject* Object)
    {
        if (Object && Object->IsA(T::StaticClass()))
            return static_cast<T*>(Object);
        return nullptr;
    }

    inline void Serialize(NObject* Obj, FArchive& Ar)
    {
        Obj->Serialize(Ar);
    }

    inline void Deserialize(NObject* Obj, FArchive& Ar)
    {
        Obj->Deserialize(Ar);
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
            if constexpr (nilou::TIsDerivedFrom<T, nilou::NAsset>::Value)
            {
                auto asset = std::static_pointer_cast<nilou::NAsset>(Object);
                Ar.Node = asset->GetPath();
            }
            else 
            {
                Object->Serialize(Ar);
            }
        }
    }
    static void Deserialize(std::shared_ptr<T>& Object, FArchive& Ar) 
    { 
        if constexpr (nilou::TIsDerivedFrom<T, nilou::NAsset>::Value)
        {
            if (Ar.Node.is_string())
            {
                std::string path = Ar.Node.get<std::string>();
                Object = std::static_pointer_cast<T>(nilou::GetContentManager()->GetContentByPath(path)->shared_from_this());
            }
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
            if constexpr (nilou::TIsDerivedFrom<T, nilou::NAsset>::Value)
            {
                auto asset = static_cast<nilou::NAsset*>(Object);
                Ar.Node = asset->GetVirtualPath();
            }
            else 
            {
                Object->Serialize(Ar);
            }
        }
    }
    static void Deserialize(T*& Object, FArchive& Ar) 
    { 
        if constexpr (nilou::TIsDerivedFrom<T, nilou::NAsset>::Value)
        {
            if (Ar.Node.is_string())
            {
                std::string path = Ar.Node.get<std::string>();
                Object = static_cast<T*>(nilou::GetContentManager()->GetContentByPath(path));
            }
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