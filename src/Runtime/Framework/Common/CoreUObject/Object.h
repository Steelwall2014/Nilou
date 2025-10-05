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

    class FProperty
    {
    public:
        std::string Name;

        template <typename T>
        T* ContainerPtrToValuePtr(void* ContainerPtr, int32 ArrayIndex=0) const
        {
            return static_cast<T*>(ContainerPtrToValuePtrInternal(ContainerPtr, ArrayIndex));
        }

        void* ContainerPtrToValuePtrInternal(void* ContainerPtr, int32 ArrayIndex=0) const;

        virtual void SerializeItem(FArchive& Ar, void* Value) = 0;

    private:
        int32 Offset_Internal;

        int32 ElementSize;

        template<typename T>
        friend class TClassRegistry;
        friend class FClassRegistryBase;

    };

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
    inline void Serialize(FArchive& Ar, bool Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, int32 Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, float Value) { SerializePrivate::SerializeNumeric(Ar, Value); }
    inline void Serialize(FArchive& Ar, double Value) { SerializePrivate::SerializeNumeric(Ar, Value); }

    template<typename T>
    class TProperty_Numeric : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            Serialize(Ar[Name], *static_cast<T*>(Value));
        }
    };
    using FBoolProperty = TProperty_Numeric<bool>;
    using FIntProperty = TProperty_Numeric<int32>;
    using FFloatProperty = TProperty_Numeric<float>;
    using FDoubleProperty = TProperty_Numeric<double>;

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
    class FStrProperty : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            Serialize(Ar[Name], *static_cast<std::string*>(Value));
        }
    };

    class FArrayProperty : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar[Name], Value);
        }

        std::function<void(FArchive&,void*)> ItemSerializer;
    };

    class FMapProperty : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar[Name], Value);
        }

        std::function<void(FArchive&,void*)> ItemSerializer;
    };

    class FVectorProperty : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar[Name], Value);
        }

        std::function<void(FArchive&,void*)> ItemSerializer;
    };

    class FQuatProperty : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar[Name], Value);
        }

        std::function<void(FArchive&,void*)> ItemSerializer;
    };

    class FSetProperty : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar[Name], Value);
        }

        std::function<void(FArchive&,void*)> ItemSerializer;
    };

    class FStructProperty : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override;

        NClass* Struct;
    };

    class FObjectProperty : public FProperty
    {
    public:
        virtual void SerializeItem(FArchive& Ar, void* Value) override;
    };

    enum class EObjectFlags : uint32
    {
        NoFlags	                    =0x00000000,
        NeedLoad		            =0x00000400,
        NeedPostLoad                =0x00001000,
        RF_LoadCompleted			=0x00200000,
    };
    ENUM_CLASS_FLAGS(EObjectFlags);

    class NObject : public std::enable_shared_from_this<NObject>
    {

    private:
        template<typename T> 
        friend class TClassRegistry;
        static NClass* Z_StaticClass;
        friend struct FIntrinsicClassRegistry;
    public:
        virtual NClass *GetClass() const { return StaticClass(); }
        static NClass *StaticClass() { return Z_StaticClass; }
        virtual void Serialize(FArchive& Ar);
        virtual void PostLoad();

    public:
        NFUNCTION()
        bool IsA(const NClass *Class);

        std::string GetName() const
        {
            return NamePrivate;
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

        friend void InitializeObject(std::shared_ptr<NObject> Object, const std::string& Name, NObject* Outer, NClass* Class);
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

    inline void Serialize(FArchive& Ar, NObject*& Object)
    {
        nlohmann::json& Node = Ar.GetNode();
        if (Ar.IsLoading())
        {
            std::string ObjectPathName = Node.get<std::string>();
            Object = LoadObject(ObjectPathName);
        }
    }

    struct FStaticConstructObjectParameters
    {
        const NClass* Class;

        NObject* Outer;

        std::string Name;
    };
    NObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params);

    void ForEachObjectWithPackage(const NPackage* Package, std::function<bool(NObject*)> Operation, bool bIncludeNestedObjects = true);
    TArray<NObject *> GetObjectsWithPackage(const class NPackage* Package, bool bIncludeNestedObjects = true);

}
