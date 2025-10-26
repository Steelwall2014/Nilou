#pragma once
#include <json/json.hpp>
#include "Common/Containers/Array.h"
#include "Common/Containers/Set.h"
#include "Templates/ObjectMacros.h"
#include "SerializeHelper.h"
#include "Archive.h"
#include "Common/EnumClassFlags.h"

// #include "Object.generated.h"
namespace nilou {

    class NObject;
    class NClass;
    class NPackage;


    namespace SerializePrivate {
        template <typename T>
        void SerializeNumeric(FArchive& Ar, T& Value)
        {
            nlohmann::json& Node = Ar.GetNode();
            if (Ar.IsLoading())
            {
                Value = Node.get<T>();
            }
            else
            {
                Node = Value;
            }
        }
    }
    inline void Serialize(FArchive& Ar, bool& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, int8& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, int16& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, int32& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, int64& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, uint8& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, uint16& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, uint32& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, uint64& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, float& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, double& Value) { SerializePrivate::SerializeNumeric(Ar, Value); }

    inline void Serialize(FArchive& Ar, std::string& Value)
    {
        nlohmann::json& Node = Ar.GetNode();
        if (Ar.IsLoading())
        {
            Value = Node.get<std::string>();
        }
        else
        {
            Node = Value;
        }
    }

    enum class EObjectFlags : uint32
    {
        NoFlags	                    =0x00000000,
        ClassDefaultObject		    =0x00000010,
        NeedLoad		            =0x00000400,
        NeedPostLoad                =0x00001000,
        WasLoaded			        =0x00080000,
    };
    ENUM_CLASS_FLAGS(EObjectFlags);

    struct FObjectInitializer
    {
        NClass* Class;
        std::string Name;
        NObject* Outer;
        EObjectFlags Flags;

        static FObjectInitializer& Get();
    };

    class NObject
    {

    private:
        template<typename T> 
        friend class TClassRegistry;
        friend class NClass;
        static NClass* Z_StaticClass;
        friend void InitUObject();
    public:
        static NClass *StaticClass() { return Z_StaticClass; }
        virtual void Serialize(FArchive& Ar);
        virtual void PostLoad();

    public:
        NObject(const FObjectInitializer& Initializer=FObjectInitializer::Get());
        virtual ~NObject();

        NFUNCTION()
        bool IsA(const NClass *Class) const;

        template <typename T>
        bool IsA() const
        {
            return IsA(T::StaticClass());
        }

        std::string GetName() const
        {
            return NamePrivate;
        }

        NClass *GetClass() const 
        { 
            return ClassPrivate; 
        }

        std::string GetPathName() const;

        NFUNCTION()
        void Rename(const std::string& NewName)
        {
            NamePrivate = NewName;
        }

        NPackage* GetPackage() const;

        NObject* GetOuter() const { return OuterPrivate; }

        /** Note by Steelwall2014: 
        * This is a temporary function used for serialization.
        * It will be removed after full reflection is implemented.
        */
        virtual void GetObjectReferences(TSet<NObject*>& OutReferences) const;

        void SetFlags(EObjectFlags FlagsToAdd)
        {
            EObjectFlags OldFlags = EObjectFlags::NoFlags;
            EObjectFlags NewFlags = EObjectFlags::NoFlags;
            do 
            {
                NewFlags = OldFlags | FlagsToAdd;
            }
            while( !ObjectFlags.compare_exchange_weak(OldFlags, NewFlags) );
        }

        void ClearFlags(EObjectFlags FlagsToClear)
        {
            EObjectFlags OldFlags = EObjectFlags::NoFlags;
            EObjectFlags NewFlags = EObjectFlags::NoFlags;
            do 
            {
                NewFlags = OldFlags & (~FlagsToClear);
            }
            while( !ObjectFlags.compare_exchange_weak(OldFlags, NewFlags) );
        }

        bool HasAnyFlags(EObjectFlags FlagsToCheck) const
        {
            return (ObjectFlags & FlagsToCheck) != EObjectFlags::NoFlags;
        }

        bool HasAllFlags(EObjectFlags FlagsToCheck) const
        {
            return (ObjectFlags & FlagsToCheck) == FlagsToCheck;
        }

        void ConditionalPostLoad();

        void MarkPackageDirty();

    private:
        std::atomic<EObjectFlags> ObjectFlags;

        NClass* ClassPrivate;

        std::string NamePrivate;

        NObject* OuterPrivate;
    };

    template <class T>
    T* Cast(NObject* Object)
    {
        if (Object && Object->IsA(T::StaticClass()))
            return static_cast<T*>(Object);
        return nullptr;
    }

    NObject* NewObject(NObject* Outer, const std::string& Name, NClass* Class);
    template <typename T>
    T* NewObject(NObject* Outer, const std::string& Name, NClass* Class=T::StaticClass())
    {
        return Cast<T>(NewObject(Outer, Name, Class));
    }

    NObject* LoadObject(const std::string& Path);
    template <typename T>
    T* LoadObject(const std::string& Path)
    {
        return Cast<T>(LoadObject(Path));
    }

    NObject* FindObject(const std::string& Path);
    template <typename T>
    T* FindObject(const std::string& Path)
    {
        return Cast<T>(FindObject(Path));
    }
    NObject* FindObject(NObject* Outer, const std::string& Name);
    template <typename T>
    T* FindObject(NObject* Outer, const std::string& Name)
    {
        return Cast<T>(FindObject(Outer, Name));
    }

    NPackage* LoadPackage(const std::string& Name);
    NPackage* FindPackage(const std::string& Name);
    NPackage* CreatePackage(const std::string& Name);
    NPackage* GetTransientPackage();

    struct FStaticConstructObjectParameters
    {
        const NClass* Class;

        NObject* Outer;

        std::string Name;
        
        EObjectFlags Flags;
    };
    NObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params);

    void ForEachObjectWithPackage(const NPackage* Package, std::function<bool(NObject*)> Operation, bool bIncludeNestedObjects = true);
    TArray<NObject *> GetObjectsWithPackage(const class NPackage* Package, bool bIncludeNestedObjects = true);

}
