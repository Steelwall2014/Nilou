#include "Common/Containers/Map.h"
#include "Object.h"
#include "Package.h"
#include "Class.h"
#include "Serialization/AsyncLoading.h"

namespace nilou {

void* FProperty::ContainerPtrToValuePtrInternal(void* ContainerPtr, int32 ArrayIndex) const
{
    return (uint8*)ContainerPtr + Offset_Internal + ElementSize * ArrayIndex;
}

class FUObjectHashTables
{
public:
    std::mutex CriticalSection;
    TMap<std::string, std::shared_ptr<NObject>> ObjectMap;
    /** Map of object to the objects they contain */
    TMap<NObject*, TArray<NObject*>> ObjectOuterMap;

    static FUObjectHashTables& Get()
    {
        static FUObjectHashTables Instance;
        return Instance;
    }
};

NObject* NewObject(NObject* Outer, const std::string& Name, NClass* Class)
{
    if (!Outer && !Class->IsChildOf(NPackage::StaticClass()))
    {
        return nullptr;
    }
    NPackage* Package = Outer ? Cast<NPackage>(Outer->GetOuter()) : nullptr;
    NObject* Object = FindObject(Outer, Name);
    if (Object) return Object;

    FStaticConstructObjectParameters Params;
    Params.Class = Class;
    Params.Outer = Outer;
    Params.Name = Name;
    Object = StaticConstructObject_Internal(Params);
    return Object;
}

NObject* LoadObject(const std::string& Path)
{
    NObject* Object = FindObject(Path);
    if (Object)
        return Object;

    constexpr std::string Delimiter = ".";
    auto LastDot = Path.find_last_of(Delimiter);
    std::string PackageName = Path.substr(0, LastDot);
    std::string ObjectName = Path.substr(LastDot + 1);

    NPackage* Package = LoadPackage(PackageName);
    if (!Package) return nullptr;

    Object = FindObject(Path);
    return Object;
}

NObject* FindObject(const std::string& Path)
{
	FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
    std::lock_guard<std::mutex> Lock(ThreadHash.CriticalSection);
    auto& ObjectMap = ThreadHash.ObjectMap;
    auto Found = ObjectMap.Find(Path);
    if (Found)
        return Found->get();

    return nullptr;
}

NObject* FindObject(NObject* Outer, const std::string& Name)
{
    FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
    std::lock_guard<std::mutex> Lock(ThreadHash.CriticalSection);
    auto& ObjectOuterMap = ThreadHash.ObjectOuterMap;
    auto Found = ObjectOuterMap.Find(Outer);
    if (Found)
    {
        for (auto Object : *Found)
        {
            if (Object->GetName() == Name)
                return Object;
        }
    }

    return nullptr;
}

NPackage* LoadPackage(const std::string& Name)
{
    NObject* Object = FindObject(Name);
    if (Object) return Cast<NPackage>(Object);

    int32 RequestId = FAsyncLoadingThread::Get().LoadPackage(Name);
    FAsyncLoadingThread::Get().FlushAsyncLoading( { RequestId } );

    Object = FindObject(Name);
    if (Object) return Cast<NPackage>(Object);

    return nullptr;
}

NPackage* FindPackage(const std::string& Name)
{
    NObject* Object = FindObject(Name);
    if (Object) return Cast<NPackage>(Object);

    return nullptr;
}

NPackage* CreatePackage(const std::string& Name)
{
    NPackage* Package = LoadPackage(Name);
    if (Package) return Package;
    return NewObject<NPackage>(nullptr, Name);
}

NObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params)
{    
    NObject* Object = Params.Class->CreateObject(Params.Name, Params.Outer);
    return Object;
}

void InitializeObject(std::shared_ptr<NObject> Object, const std::string& Name, NObject* Outer)
{
    Object->NamePrivate = Name;
    Object->OuterPrivate = Outer;
    FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
    std::lock_guard<std::mutex> Lock(ThreadHash.CriticalSection);
    ThreadHash.ObjectMap.Add(Name, Object);
    NPackage* Package = Object->GetPackage();
    if (Package)
    {
        ThreadHash.ObjectOuterMap.FindOrAdd(Package).Add(Object.get());
    }
}

void ForEachObjectWithPackage(const NPackage* Package, std::function<bool(NObject*)> Operation, bool bIncludeNestedObjects)
{
	FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
    std::lock_guard<std::mutex> Lock(ThreadHash.CriticalSection);
    TArray<TArray<NObject*>*> AllInners;

	// Add the object bucket that have this package as an outer
	if (TArray<NObject*>* Inners = ThreadHash.ObjectOuterMap.Find(const_cast<NPackage*>(Package)))
	{
        AllInners.Add(Inners);
	}

    while (AllInners.Num() > 0) 
    {
        TArray<NObject*>* Inners = AllInners.Pop();
        for (NObject* Object : *Inners) 
        {
            if (!Operation(Object))
            {
                AllInners.Empty();
                break;
            }
            if (bIncludeNestedObjects)
            {
                if (TArray<NObject*>* ObjectInners = ThreadHash.ObjectOuterMap.Find(Object))
                {
                    AllInners.Add(ObjectInners);
                }
            }
        }
    }
}

TArray<NObject *> GetObjectsWithPackage(const class NPackage* Package, bool bIncludeNestedObjects)
{
    TArray<NObject*> Results;
    ForEachObjectWithPackage(Package, [&Results](NObject* Object)
	{
		Results.Add(Object);
		return true;
    }, bIncludeNestedObjects);
    return Results;
}

bool NObject::IsA(const NClass *Class)
{
    return GetClass()->IsChildOf(Class);
}

std::string NObject::GetPathName() const
{
    std::string PathName;
    const NObject* Top = this;
    for (;;)
    {
        if (Top->GetOuter() == nullptr)
        {
            PathName = Top->GetName() + PathName;
            break;
        }
        PathName = "." + Top->GetName() + PathName;
        Top = Top->GetOuter();
    }
    return PathName;
}

NPackage* NObject::GetPackage() const
{
    const NObject* Top = this;
    for (;;)
    {
        if (Top->GetOuter() == nullptr)
        {
            return Cast<NPackage>(const_cast<NObject*>(Top));
        }
        Top = Top->GetOuter();
    }
    return nullptr;
}

void NObject::GetObjectReferences(TSet<NObject*>& OutReferences) const
{
    OutReferences.Add(GetClass());
}

void NObject::Serialize(FArchive& Ar)
{
    TArray<FProperty*> Properties = GetClass()->GetProperties(true);
    for (FProperty* Property : Properties)
    {
        void* Pointer = Property->ContainerPtrToValuePtrInternal(this);
        Property->SerializeItem(Ar[Property->Name], Pointer);
    }
}

void NObject::PostLoad()
{

}

void NObject::ConditionalPostLoad()
{
    if (HasAnyFlags(EObjectFlags::NeedPostLoad))
    {
        ClearFlags(EObjectFlags::NeedPostLoad);
        PostLoad();
    }
}

void NObject::MarkPackageDirty()
{
    NPackage* Package = GetPackage();
    if (Package)
    {
        bool bIsDirty = Package->IsDirty();
        if (!bIsDirty)
        {
            Package->SetDirtyFlag(true);
        }
    }
}

NPackage* CreateNativeClassNPackage()
{
    std::shared_ptr<NPackage> Object = std::make_shared<NPackage>();
    InitializeObject(Object, "/Script/Engine", nullptr);
    return Object.get();
}

NClass* CreateNativeClassNClass()
{
    std::shared_ptr<NClass> Object = std::make_shared<NClass>();
    InitializeObject(Object, "Class", CreateNativeClassNPackage());
    Object->SetClassFlags(EClassFlags::Intrinsic);
    return Object.get();
}

NObject* NClass::CreateObject(const std::string& Name, NObject* Outer) const
{
    void* Memory = malloc(Size);
    ClassConstructor(Memory);
    std::shared_ptr<NObject> Object(static_cast<NObject*>(Memory));
    InitializeObject(Object, Name, Outer); 
    return Object.get();
}

TArray<FClassRegistryBase*> Registrations;
FClassRegistryBase::FClassRegistryBase(
    EMetaClass InMetaClass, 
    const std::string& InName, 
    NClass* InSuperClass, 
    int32 Size, 
    EClassFlags InClassFlags, 
    std::function<void(void*)> InClassConstructor)
{
    static NPackage* NativeClassPackage = CreateNativeClassNPackage();
    static NClass* NativeClassClass = CreateNativeClassNClass();
    std::string Name = RemoveNamespace(InName);
    Name = RemovePrefix(InMetaClass, Name);
    FStaticConstructObjectParameters Params;
    Params.Class = NativeClassClass;
    Params.Outer = NativeClassPackage;
    Params.Name = Name;
    Class = Cast<NClass>(StaticConstructObject_Internal(Params));
    Class->SuperStruct = InSuperClass;
    Class->ClassConstructor = InClassConstructor;
    Class->SetClassFlags(InClassFlags);
    Registrations.Add(this);
}

void FClassRegistryBase::DeferredConstructFProperty()
{
    for (FClassRegistryBase* Registry : Registrations)
    {
        Registry->ConstructFProperty();
    }
}

std::string FClassRegistryBase::RemoveNamespace(const std::string& Name)
{
    auto pos = Name.find_last_of("::");
    return pos != std::string::npos ? Name.substr(pos + 1) : Name;
}

std::string FClassRegistryBase::RemovePrefix(EMetaClass InMetaClass, const std::string& Name)
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

std::unique_ptr<NClass> NObject::Z_StaticClass = nullptr;
NClass *NObject::GetClass() const 
{ 
    return NObject::StaticClass(); 
}
NClass *NObject::StaticClass()
{
    return NObject::Z_StaticClass.get();
}
template<>
struct TClassRegistry<NObject> : public FClassRegistryBase
{
    TClassRegistry<NObject>() 
        : FClassRegistryBase(
            EMetaClass::Object, 
            "nilou::NObject", 
            nullptr, 
            sizeof(NObject),
            EClassFlags::Native | EClassFlags::Intrinsic, 
            [](void* Memory) { new(Memory) NObject(); }) 
    {
        ConstructFProperty = [this]()
        {
            FStrProperty* Property = new FStrProperty();
            Property->Name = "Name";
            Property->Offset_Internal = offsetof(NObject, NamePrivate);
            Property->ElementSize = sizeof(static_cast<NObject*>(nullptr)->NamePrivate);
            Class->Properties.Add(Property);
        };
    }
};
TClassRegistry<NObject> ClassRegistry_NObject;

}
