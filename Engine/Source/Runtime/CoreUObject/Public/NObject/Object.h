#pragma once
#include <nlohmann/json.hpp>
#include "Containers/Array.h"
#include "Containers/Set.h"
#include "NObject/ObjectMacros.h"
#include "Serialization/Archive.h"
#include "Misc/EnumClassFlags.h"

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

    struct COREUOBJECT_API FObjectInitializer
    {
        NClass* Class = nullptr;
        std::string Name;
        NObject* Outer = nullptr;
        EObjectFlags Flags = EObjectFlags::NoFlags;

        static FObjectInitializer& Get();
    };

    class COREUOBJECT_API NObject
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
        NObject(const NObject&) = delete;
        NObject& operator=(const NObject&) = delete;
        NObject(NObject&&) = delete;
        NObject& operator=(NObject&&) = delete;

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

        void Rename(const std::string& NewName);

        NPackage* GetPackage() const;

        NObject* GetOuter() const { return OuterPrivate; }

        /** Note by Steelwall2014: 
        * This is a temporary function used for serialization.
        * It will be removed after full reflection is implemented.
        */
        virtual void GetObjectReferences(TSet<NObject*>& OutReferences) const;

        void SetFlags(EObjectFlags FlagsToAdd)
        {
            EObjectFlags OldFlags = ObjectFlags.load();
            EObjectFlags NewFlags = EObjectFlags::NoFlags;
            do 
            {
                NewFlags = OldFlags | FlagsToAdd;
            }
            while( !ObjectFlags.compare_exchange_weak(OldFlags, NewFlags) );
        }

        void ClearFlags(EObjectFlags FlagsToClear)
        {
            EObjectFlags OldFlags = ObjectFlags.load();
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

    template <class T>
    const T* Cast(const NObject* Object)
    {
        if (Object && Object->IsA(T::StaticClass()))
            return static_cast<const T*>(Object);
        return nullptr;
    }

    COREUOBJECT_API NObject* NewObject(NObject* Outer, const std::string& Name, NClass* Class);
    template <typename T>
    T* NewObject(NObject* Outer, const std::string& Name, NClass* Class=T::StaticClass())
    {
        return Cast<T>(NewObject(Outer, Name, Class));
    }

    COREUOBJECT_API NObject* LoadObject(const std::string& Path);
    template <typename T>
    T* LoadObject(const std::string& Path)
    {
        return Cast<T>(LoadObject(Path));
    }

    COREUOBJECT_API NObject* FindObject(const std::string& Path);
    template <typename T>
    T* FindObject(const std::string& Path)
    {
        return Cast<T>(FindObject(Path));
    }
    COREUOBJECT_API NObject* FindObject(NObject* Outer, const std::string& Name);
    template <typename T>
    T* FindObject(NObject* Outer, const std::string& Name)
    {
        return Cast<T>(FindObject(Outer, Name));
    }

    COREUOBJECT_API NPackage* LoadPackage(const std::string& Name);
    COREUOBJECT_API NPackage* FindPackage(const std::string& Name);
    COREUOBJECT_API NPackage* CreatePackage(const std::string& Name);
    COREUOBJECT_API NPackage* GetTransientPackage();

    struct FStaticConstructObjectParameters
    {
        const NClass* Class;

        NObject* Outer;

        std::string Name;
        
        EObjectFlags Flags;
    };
    COREUOBJECT_API NObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params);

    COREUOBJECT_API TArray<NObject *> GetObjectsWithPackage(const class NPackage* Package, bool bIncludeNestedObjects = true);

}
