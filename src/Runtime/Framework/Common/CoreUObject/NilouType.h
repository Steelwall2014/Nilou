#pragma once
#include "Object.h"
#include "Class.h"

namespace nilou {

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
        static FFieldClass *StaticClass(); \
        TClass(const std::string& InName) : Super(InName) { ClassPrivate = StaticClass(); }

#define IMPLEMENT_FIELD(TClass) \
    FFieldClass *TClass::StaticClass() \
    { \
        static FFieldClass StaticFieldClass(#TClass, Super::StaticClass()); \
        return &StaticFieldClass; \
    }

class FProperty
{
public:
    FProperty(const std::string& InName) : Name(InName) {}
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

protected:
    FFieldClass* ClassPrivate = nullptr;

private:
    int32 Offset_Internal;

    int32 ElementSize;

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
protected:
    TProperty_Numeric(const std::string& InName) : FNumericProperty(InName) {}
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


#if __cplusplus >= 202002L
template <typename T>
concept HasStaticClass = requires {
    { T::StaticClass() } -> std::same_as<NClass*>;
};
template <typename T>
struct THasStaticClass { static constexpr bool Value = HasStaticClass<T>; };

template <typename T>
concept HasStaticEnum = requires {
    { StaticEnum<T>() } -> std::same_as<NClass*>;
};
template <typename T>
struct THasStaticEnum { static constexpr bool Value = HasStaticEnum<T>; };
#else
template <typename T>
struct THasStaticClass 
{ 
private:
    template <typename U>
    static constexpr auto Test(U*) -> decltype(U::StaticClass(), std::true_type{});
    template <typename U>
    static constexpr std::false_type Test(...);
public:
    static constexpr bool Value = decltype(Test<T>(nullptr))::value; 
};

template <typename T>
struct THasStaticEnum 
{ 
private:
    template <typename U>
    static constexpr auto Test(U*) -> decltype(StaticEnum<U>(), std::true_type{});
    template <typename U>
    static constexpr std::false_type Test(...);
public:
    static constexpr bool Value = decltype(Test<T>(nullptr))::value; 
};
#endif

struct FClassRegistryBase
{
    struct NullSuperClass { static NClass* StaticClass() { return nullptr; } };

    FClassRegistryBase(EMetaClass InMetaClass, const std::string& InName, NClass* InSuperClass, int32 InSize, EClassFlags InClassFlags, std::function<void(void*)> InDefaultClassConstructor);
    
    EMetaClass MetaClass;

    NClass* Class;

    static TArray<FClassRegistryBase*> Registrations;

    template<typename T> 
    struct TConstructProperty 
    { 
        static FProperty* Construct(const std::string& Name) 
        {
            if constexpr (std::is_pointer_v<T> && TIsDerivedFrom<std::remove_pointer_t<T>, NObject>::Value)
            {
                return new FObjectProperty(Name);
            }
            else if constexpr (std::is_enum_v<T> && THasStaticEnum<T>::Value)
            {
                FEnumProperty* Property = new FEnumProperty(Name);
                Property->Enum = StaticEnum<T>();
                return Property;
            }
            else if constexpr (THasStaticClass<T>::Value)
            {
                FStructProperty* Property = new FStructProperty(Name);
                Property->Struct = T::StaticClass();
                return Property;
            }
            else 
            {
                static_assert(false, "T is not a supported reflection type.");
                return nullptr;
            }
        } 
    };
    
    template<> struct TConstructProperty<std::string> { static FProperty* Construct(const std::string& Name) { return new FStrProperty(Name); } };
    template<> struct TConstructProperty<int8> { static FProperty* Construct(const std::string& Name) { return new FInt8Property(Name); } };
    template<> struct TConstructProperty<int16> { static FProperty* Construct(const std::string& Name) { return new FInt16Property(Name); } };
    template<> struct TConstructProperty<int32> { static FProperty* Construct(const std::string& Name) { return new FInt32Property(Name); } };
    template<> struct TConstructProperty<int64> { static FProperty* Construct(const std::string& Name) { return new FInt64Property(Name); } };
    template<> struct TConstructProperty<uint8> { static FProperty* Construct(const std::string& Name) { return new FUInt8Property(Name); } };
    template<> struct TConstructProperty<uint16> { static FProperty* Construct(const std::string& Name) { return new FUInt16Property(Name); } };
    template<> struct TConstructProperty<uint32> { static FProperty* Construct(const std::string& Name) { return new FUInt32Property(Name); } };
    template<> struct TConstructProperty<uint64> { static FProperty* Construct(const std::string& Name) { return new FUInt64Property(Name); } };
    template<> struct TConstructProperty<bool> { static FProperty* Construct(const std::string& Name) { return new FBoolProperty(Name); } };
    template<> struct TConstructProperty<float> { static FProperty* Construct(const std::string& Name) { return new FFloatProperty(Name); } };
    template<> struct TConstructProperty<double> { static FProperty* Construct(const std::string& Name) { return new FDoubleProperty(Name); } };
    template<typename T> struct TConstructProperty<TArray<T>>
    { 
        static FProperty* Construct(const std::string& Name) 
        { 
            FArrayProperty* Property = new FArrayProperty(Name);
            Property->Inner = TConstructProperty<T>::Construct(Name + ".Inner");
            Property->GetNum = [](const void* Array) { return reinterpret_cast<const TArray<T>*>(Array)->Num(); };
            Property->GetItem = [](void* Array, size_t Index) { return &(reinterpret_cast<TArray<T>*>(Array)->GetData()[Index]); };
            Property->ItemIdentical = &FArrayProperty::ItemIdenticalTemplate<T>;
            Property->ItemSerializer = &FArrayProperty::ItemSerializerTemplate<T>;
            return Property;
        } 
    };
    template<> struct TConstructProperty<TArray<uint8>>
    { 
        static FProperty* Construct(const std::string& Name) 
        { 
            FArrayProperty* Property = new FArrayProperty(Name);
            Property->Inner = TConstructProperty<uint8>::Construct(Name + ".Inner");
            Property->GetNum = [](const void* Array) { return reinterpret_cast<const TArray<uint8>*>(Array)->Num(); };
            Property->GetItem = [](void* Array, size_t Index) { return &(reinterpret_cast<TArray<uint8>*>(Array)->GetData()[Index]); };
            Property->ItemIdentical = &FArrayProperty::ItemIdenticalTemplate<uint8>;
            Property->ItemSerializer = [](FArchive& Ar, void* Value, FProperty* Inner) 
            {
                TArray<uint8>& Array = *reinterpret_cast<TArray<uint8>*>(Value);
                nlohmann::json& Node = Ar.GetNode();
                if (Ar.IsLoading())
                {
                    std::string encoded_data = Node.get<std::string>();
                    std::string data = base64_decode(encoded_data.data(), encoded_data.size());
                    Array.SetNum(data.size());
                    std::memcpy(Array.GetData(), data.data(), data.size());
                }
                else
                {
                    Node = base64_encode(Array.GetData(), Array.Num());
                }
            };
            return Property;
        } 
    };
    template<typename K, typename V> struct TConstructProperty<TMap<K, V>> 
    { 
        static FProperty* Construct(const std::string& Name) 
        { 
            FMapProperty* Property = new FMapProperty(Name);
            Property->KeyProp = TConstructProperty<K>::Construct(Name + ".KeyProp");
            Property->ValueProp = TConstructProperty<V>::Construct(Name + ".ValueProp");
            Property->GetNum = [](const void* pMap) { return reinterpret_cast<const TMap<K, V>*>(pMap)->Num(); };
            Property->GetPairs = [](const void* pMap) 
            { 
                const TMap<K, V>& Map = *reinterpret_cast<const TMap<K, V>*>(pMap);
                TArray<void*> Pairs;
                for (auto& Pair : Map)
                {
                    Pairs.Add((void*)&Pair);
                }
                return Pairs;
            };
            Property->PairGetKey = [](void* Pair) -> void* { return (void*)&(reinterpret_cast<TMap<K, V>::value_type*>(Pair)->first); };
            Property->PairGetValue = [](void* Pair) -> void* { return &(reinterpret_cast<TMap<K, V>::value_type*>(Pair)->second); };
            Property->ItemIdentical = &FMapProperty::ItemIdenticalTemplate<K, V>;
            Property->ItemSerializer = &FMapProperty::ItemSerializerTemplate<K, V>;
            return Property;
        } 
    };
    template<typename T> struct TConstructProperty<TSet<T>> 
    { 
        static FProperty* Construct(const std::string& Name) 
        { 
            FSetProperty* Property = new FSetProperty(Name);
            Property->Inner = TConstructProperty<T>::Construct(Name + ".Inner");
            Property->GetItems = [](const void* pSet)
            {
                const TSet<T>& Set = *reinterpret_cast<const TSet<T>*>(pSet);
                TArray<void*> Items;
                for (auto& Item : Set)
                {
                    Items.Add((void*)&Item);
                }
                return Items;
            };
            Property->GetNum = [](const void* Set) { return reinterpret_cast<const TSet<T>*>(Set)->Num(); };
            Property->ItemIdentical = &FSetProperty::ItemIdenticalTemplate<T>;
            Property->ItemSerializer = &FSetProperty::ItemSerializerTemplate<T>;
            return Property;
        } 
    };
    template<typename T> struct TConstructProperty<TVector2<T>> 
    { 
        static FProperty* Construct(const std::string& Name) 
        { 
            FVectorProperty* Property = new FVectorProperty(Name);
            Property->ItemIdentical = &FVectorProperty::ItemIdenticalTemplate<TVector2<T>>;
            Property->ItemSerializer = &FVectorProperty::ItemSerializerTemplate<TVector2<T>>;
            return Property;
        } 
    };
    template<typename T> struct TConstructProperty<TVector<T>> 
    { 
        static FProperty* Construct(const std::string& Name) 
        { 
            FVectorProperty* Property = new FVectorProperty(Name);
            Property->ItemIdentical = &FVectorProperty::ItemIdenticalTemplate<TVector<T>>;
            Property->ItemSerializer = &FVectorProperty::ItemSerializerTemplate<TVector<T>>;
            return Property;
        } 
    };
    template<typename T> struct TConstructProperty<TVector4<T>> 
    { 
        static FProperty* Construct(const std::string& Name) 
        { 
            FVectorProperty* Property = new FVectorProperty(Name);
            Property->ItemIdentical = &FVectorProperty::ItemIdenticalTemplate<TVector4<T>>;
            Property->ItemSerializer = &FVectorProperty::ItemSerializerTemplate<TVector4<T>>;
            return Property;
        } 
    };
    template<typename T> struct TConstructProperty<TQuat<T>> 
    { 
        static FProperty* Construct(const std::string& Name) 
        { 
            FQuatProperty* Property = new FQuatProperty(Name);
            Property->ItemIdentical = &FQuatProperty::ItemIdenticalTemplate<TQuat<T>>;
            Property->ItemSerializer = &FQuatProperty::ItemSerializerTemplate<TQuat<T>>;
            return Property;
        } 
    };

    template <typename T, typename U>
    size_t offset_of(T U::*Member)
    {
        return reinterpret_cast<size_t>(
            &(reinterpret_cast<U*>(0)->*Member)
        );
    }

    TArray<std::function<void()>> ConstructFProperty;

    template <typename T, typename U>
    void AddProperty(const std::string& Name, T U::*Member)
    {
        ConstructFProperty.Add([this, Name, Member]() {
            auto Property = TConstructProperty<T>::Construct(Name);
            Property->Offset_Internal = offset_of(Member); \
            Property->ElementSize = sizeof(T); \
            Class->Properties.Add(Property);
        });
    }
    
    static void DeferredConstructFProperty()
    {
        for (FClassRegistryBase* Registry : Registrations)
        {
            for (auto& Constructor : Registry->ConstructFProperty)
            {
                Constructor();
            }
        }
    }
    
    std::string RemoveNamespace(const std::string& Name)
    {
        auto pos = Name.find_last_of("::");
        return pos != std::string::npos ? Name.substr(pos + 1) : Name;
    }
    
    std::string RemovePrefix(EMetaClass InMetaClass, const std::string& Name)
    {
        if (InMetaClass == EMetaClass::Struct)
        {
            Ncheck(Name[0] == 'F');
            return Name.substr(1);
        }
        else if (InMetaClass == EMetaClass::Object)
        {
            Ncheck(Name[0] == 'N' || Name[0] == 'U' || Name[0] == 'A');
            return Name.substr(1);
        }
        return Name;
    }

};


template <typename T>
struct TClassRegistry;

}