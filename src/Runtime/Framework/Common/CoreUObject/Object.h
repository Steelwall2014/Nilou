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

    NObject* LoadObject(const std::string& Path);
    NObject* FindObject(const std::string& Path);

    class NPackage* LoadPackage(const std::string& Path);
}
