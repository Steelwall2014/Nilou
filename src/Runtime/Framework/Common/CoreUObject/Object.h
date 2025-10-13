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

    class FFieldClass
    {
    public:
        FFieldClass(const std::string& InName, FFieldClass *InSuperStruct) 
            : NamePrivate(InName), SuperStruct(InSuperStruct) 
        { }

        bool IsChildOf(const FFieldClass *BaseClass) const
        {
            if (BaseClass == nullptr)
            {
                return false;
            }
            bool bResult = false;
            for (const FFieldClass* Temp = this; Temp; Temp = Temp->GetSuperStruct())
            {
                if (Temp == BaseClass)
                {
                    bResult = true;
                    break;
                }
            }
            return bResult;
        }

        std::string GetName() const { return NamePrivate; }

        FFieldClass *GetSuperStruct() const { return SuperStruct; }

    private:
        std::string NamePrivate;

        FFieldClass* SuperStruct = nullptr;
    };

    #define DECLARE_FIELD_API(TClass, TSuperClass) \
        public: \
            using Super = TSuperClass; \
            static FFieldClass *StaticClass();

    #define IMPLEMENT_FIELD(TClass) \
        FFieldClass *TClass::StaticClass() \
        { \
            static FFieldClass StaticFieldClass(#TClass, Super::StaticClass()); \
            return &StaticFieldClass; \
        }

    class FProperty
    {
    public:
        std::string Name;

        static FFieldClass *StaticClass();

        template <typename T>
        T* ContainerPtrToValuePtr(void* ContainerPtr, int32 ArrayIndex=0) const
        {
            return static_cast<T*>(ContainerPtrToValuePtrInternal(ContainerPtr, ArrayIndex));
        }
        template <typename T>
        const T* ContainerPtrToValuePtr(const void* ContainerPtr, int32 ArrayIndex=0) const
        {
            return static_cast<T*>(ContainerPtrToValuePtrInternal(ContainerPtr, ArrayIndex));
        }

        void* ContainerPtrToValuePtrInternal(void* ContainerPtr, int32 ArrayIndex=0) const;
        const void* ContainerPtrToValuePtrInternal(const void* ContainerPtr, int32 ArrayIndex=0) const;

        virtual bool Identical(const void* A, const void* B) const = 0;
        virtual void SerializeItem(FArchive& Ar, void* Value) = 0;

        FFieldClass* GetClass() const { return ClassPrivate; }

        bool IsA(const FFieldClass* Class) const { return ClassPrivate->IsChildOf(Class); }
        template <typename T>
        bool IsA() const { return IsA(T::StaticClass()); }

    private:
        int32 Offset_Internal;

        int32 ElementSize;

        FFieldClass* ClassPrivate = nullptr;

        template<typename T>
        friend class TClassRegistry;
        friend class FClassRegistryBase;

    };

    template <typename T>
    T* CastField(FProperty* Property)
    {
        if (Property->IsA<T>())
        {
            return static_cast<T*>(Property);
        }
        return nullptr;
    }

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

    class FNumericProperty : public FProperty
    {
        DECLARE_FIELD_API(FNumericProperty, FProperty)
    };

    template<typename T>
    class TProperty_Numeric : public FNumericProperty
    {
    public:
        virtual bool Identical(const void* A, const void* B) const override
        {
            return *static_cast<const T*>(A) == *static_cast<const T*>(B);
        }
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            Serialize(Ar, *static_cast<T*>(Value));
        }
    };
    
    class FBoolProperty : public TProperty_Numeric<bool>
    {
        DECLARE_FIELD_API(FBoolProperty, TProperty_Numeric<bool>)
    };
    class FInt8Property : public TProperty_Numeric<int8>
    {
        DECLARE_FIELD_API(FInt8Property, TProperty_Numeric<int8>)
    };
    class FInt16Property : public TProperty_Numeric<int16>
    {
        DECLARE_FIELD_API(FInt16Property, TProperty_Numeric<int16>)
    };
    class FInt32Property : public TProperty_Numeric<int32>
    {
        DECLARE_FIELD_API(FInt32Property, TProperty_Numeric<int32>)
    };
    class FInt64Property : public TProperty_Numeric<int64>
    {
        DECLARE_FIELD_API(FInt64Property, TProperty_Numeric<int64>)
    };
    class FUInt8Property : public TProperty_Numeric<uint8>
    {
        DECLARE_FIELD_API(FUInt8Property, TProperty_Numeric<uint8>)
    };
    class FUInt16Property : public TProperty_Numeric<uint16>
    {
        DECLARE_FIELD_API(FUInt16Property, TProperty_Numeric<uint16>)
    };
    class FUInt32Property : public TProperty_Numeric<uint32>
    {
        DECLARE_FIELD_API(FUInt32Property, TProperty_Numeric<uint32>)
    };
    class FUInt64Property : public TProperty_Numeric<uint64>
    {
        DECLARE_FIELD_API(FUInt64Property, TProperty_Numeric<uint64>)
    };
    class FFloatProperty : public TProperty_Numeric<float>
    {
        DECLARE_FIELD_API(FFloatProperty, TProperty_Numeric<float>)
    };
    class FDoubleProperty : public TProperty_Numeric<double>
    {
        DECLARE_FIELD_API(FDoubleProperty, TProperty_Numeric<double>)
    };

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
        DECLARE_FIELD_API(FStrProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override
        {
            return *static_cast<const std::string*>(A) == *static_cast<const std::string*>(B);
        }
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            Serialize(Ar, *static_cast<std::string*>(Value));
        }
    };

    class FArrayProperty : public FProperty
    {
        DECLARE_FIELD_API(FArrayProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override
        {
            return ItemIdentical(A, B, Inner);
        }
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar, Value, Inner);
        }

        template <typename TInner>
        static bool ItemIdenticalTemplate(const void* pA, const void* pB, FProperty* Inner)
        {
            const TArray<TInner>& A = *reinterpret_cast<const TArray<TInner>*>(pA);
            const TArray<TInner>& B = *reinterpret_cast<const TArray<TInner>*>(pB);

            if (A.Num() != B.Num())
            {
                return false;
            }

            for (int32 Index = 0; Index < A.Num(); ++Index)
            {
                if (!Inner->Identical(&A[Index], &B[Index]))
                {
                    return false;
                }
            }
            return true;
        }

        template <typename TInner>
        static void ItemSerializerTemplate(FArchive& Ar, void* Value, FProperty* Inner)
        {
            TArray<TInner>& Array = *reinterpret_cast<TArray<TInner>*>(Value);
            nlohmann::json& Node = Ar.GetNode();
            if (Ar.IsLoading())
            {
                Ncheck(Node.is_array());
                Array.SetNum(Node.size());
                for (int32 Index = 0; Index < Node.size(); ++Index)
                {
                    Inner->SerializeItem(Ar[Index], &Array[Index]);
                }
            }
            else
            {
                Node = nlohmann::json::array();
                for (int32 Index = 0; Index < Array.Num(); ++Index)
                {
                    Inner->SerializeItem(Ar[Index], &Array[Index]);
                }
            }
        }

        std::function<void*(void*, size_t)> GetItem;
        std::function<size_t(const void*)> GetNum;
        std::function<bool(const void*, const void*, FProperty*)> ItemIdentical;
        std::function<void(FArchive&, void*, FProperty*)> ItemSerializer;
        FProperty* Inner;
    };

    class FMapProperty : public FProperty
    {
        DECLARE_FIELD_API(FMapProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override
        {
            return ItemIdentical(A, B, KeyProp, ValueProp);
        }
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar, Value, KeyProp, ValueProp);
        }

        template <typename K, typename V>
        static bool ItemIdenticalTemplate(const void* pA, const void* pB, FProperty* KeyInner, FProperty* ValueInner)
        {
            const TMap<K, V>& A = *reinterpret_cast<const TMap<K, V>*>(pA);
            const TMap<K, V>& B = *reinterpret_cast<const TMap<K, V>*>(pB);

            if (A.Num() != B.Num())
            {
                return false;
            }

            for (auto& PairA : A)
            {
                bool bFound = false;
                for (auto& PairB : B)
                {
                    if (KeyInner->Identical(&PairA.first, &PairB.first) && ValueInner->Identical(&PairA.second, &PairB.second))
                    {
                        bFound = true;
                        break;
                    }
                }
                if (!bFound)
                {
                    return false;
                }
            }
            
            return true;
        }

        template <typename K, typename V>
        static void ItemSerializerTemplate(FArchive& Ar, void* M, FProperty* KeyProp, FProperty* ValueProp)
        {
            TMap<K, V>& Map = *reinterpret_cast<TMap<K, V>*>(M);
            nlohmann::json& Node = Ar.GetNode();
            if (Ar.IsLoading())
            {
                Ncheck(Node.is_array());
                Map.Empty();
                int32 Count = Node.size();
                for (int32 Index = 0; Index < Count; ++Index)
                {
                    K Key;
                    V Value;
                    KeyProp->SerializeItem(Ar[Index]["Key"], &Key);
                    ValueProp->SerializeItem(Ar[Index]["Value"], &Value);
                    Map.Add(Key, Value);
                }
            }
            else
            {
                Node = nlohmann::json::array();
                int32 Index = 0;
                for (auto& [Key, Value] : Map)
                {
                    KeyProp->SerializeItem(Ar[Index]["Key"], (void*)&Key);
                    ValueProp->SerializeItem(Ar[Index]["Value"], &Value);
                    Index++;
                }
            }
        };

        std::function<TArray<void*>(void*)> GetPairs;
        std::function<void*(void*)> PairGetKey;
        std::function<void*(void*)> PairGetValue;
        std::function<size_t(const void*)> GetNum;
        std::function<bool(const void*, const void*, FProperty*, FProperty*)> ItemIdentical;
        std::function<void(FArchive&, void*, FProperty*, FProperty*)> ItemSerializer;
        FProperty* KeyProp;
        FProperty* ValueProp;
    };

    class FSetProperty : public FProperty
    {
        DECLARE_FIELD_API(FSetProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override
        {
            return ItemIdentical(A, B, Inner);
        }
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar, Value, Inner);
        }

        template <typename T>
        static bool ItemIdenticalTemplate(const void* pA, const void* pB, FProperty* Inner)
        {
            const TSet<T>& A = *reinterpret_cast<const TSet<T>*>(pA);
            const TSet<T>& B = *reinterpret_cast<const TSet<T>*>(pB);

            if (A.Num() != B.Num())
            {
                return false;
            }

            for (auto& ValueA : A)
            {
                bool bFound = false;
                for (auto& ValueB : B)
                {
                    if (Inner->Identical(&ValueA, &ValueB))
                    {
                        bFound = true;
                        break;
                    }
                }
                if (!bFound)
                {
                    return false;
                }
            }
            return true;
        }

        template <typename T>
        static void ItemSerializerTemplate(FArchive& Ar, void* Value, FProperty* Inner)
        {
            TSet<T>& Set = *reinterpret_cast<TSet<T>*>(Value);
            nlohmann::json& Node = Ar.GetNode();
            if (Ar.IsLoading())
            {
                Ncheck(Node.is_array());
                Set.Empty();
                int32 Count = Node.size();
                for (int32 Index = 0; Index < Count; ++Index)
                {
                    T Value;
                    Inner->SerializeItem(Ar[Index], &Value);
                    Set.Add(Value);
                }
            }
            else
            {
                Node = nlohmann::json::array();
                int32 Index = 0;
                for (auto& Value : Set)
                {
                    Inner->SerializeItem(Ar[Index], (void*)&Value);
                    Index++;
                }
            }
        }
            
        std::function<TArray<void*>(void*)> GetItems;
        std::function<size_t(const void*)> GetNum;
        std::function<bool(const void*, const void*, FProperty*)> ItemIdentical;
        std::function<void(FArchive&,void*,FProperty*)> ItemSerializer;
        FProperty* Inner;
    };

    class FVectorProperty : public FProperty
    {
        DECLARE_FIELD_API(FVectorProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override
        {
            return ItemIdentical(A, B);
        }
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar, Value);
        }

        template <typename TVector>
        static bool ItemIdenticalTemplate(const void* pA, const void* pB)
        {
            const TVector& A = *reinterpret_cast<const TVector*>(pA);
            const TVector& B = *reinterpret_cast<const TVector*>(pB);
            return A == B;
        }

        template <typename TVector>
        static void ItemSerializerTemplate(FArchive& Ar, void* Value)
        {
            TVector& Vec = *reinterpret_cast<TVector*>(Value);
            Serialize(Ar, Vec);
        }

        std::function<bool(const void*, const void*)> ItemIdentical;
        std::function<void(FArchive&,void*)> ItemSerializer;
    };

    class FQuatProperty : public FProperty
    {
        DECLARE_FIELD_API(FQuatProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override
        {
            return ItemIdentical(A, B);
        }
        virtual void SerializeItem(FArchive& Ar, void* Value) override
        {
            ItemSerializer(Ar, Value);
        }

        template <typename TQuat>
        static bool ItemIdenticalTemplate(const void* pA, const void* pB)
        {
            const TQuat& A = *reinterpret_cast<const TQuat*>(pA);
            const TQuat& B = *reinterpret_cast<const TQuat*>(pB);
            return A == B;
        }

        template <typename TQuat>
        static void ItemSerializerTemplate(FArchive& Ar, void* Value)
        {
            TQuat& Quat = *reinterpret_cast<TQuat*>(Value);
            Serialize(Ar, Quat);
        }

        std::function<bool(const void*, const void*)> ItemIdentical;
        std::function<void(FArchive&,void*)> ItemSerializer;
    };

    class FStructProperty : public FProperty
    {
        DECLARE_FIELD_API(FStructProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override;
        virtual void SerializeItem(FArchive& Ar, void* Value) override;

        NClass* Struct;
    };

    class FObjectProperty : public FProperty
    {
        DECLARE_FIELD_API(FObjectProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override;
        virtual void SerializeItem(FArchive& Ar, void* Value) override;
    };

    class FEnumProperty : public FProperty
    {
        DECLARE_FIELD_API(FEnumProperty, FProperty)
    public:
        virtual bool Identical(const void* A, const void* B) const override;
        virtual void SerializeItem(FArchive& Ar, void* Value) override;

        NClass* Enum;
    };

    struct FObjectInitializer
    {
        NClass* Class;
        std::string Name;
        NObject* Outer;

        static FObjectInitializer& Get();
    };

    enum class EObjectFlags : uint32
    {
        NoFlags	                    =0x00000000,
        ClassDefaultObject		    =0x00000010,
        NeedLoad		            =0x00000400,
        NeedPostLoad                =0x00001000,
        LoadCompleted			    =0x00200000,
    };
    ENUM_CLASS_FLAGS(EObjectFlags);

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

        NFUNCTION()
        bool IsA(const NClass *Class);

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
    };
    NObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params);

    void ForEachObjectWithPackage(const NPackage* Package, std::function<bool(NObject*)> Operation, bool bIncludeNestedObjects = true);
    TArray<NObject *> GetObjectsWithPackage(const class NPackage* Package, bool bIncludeNestedObjects = true);

}
