#include "Common/Containers/Map.h"
#include "Object.h"
#include "Package.h"
#include "Class.h"
#include "Serialization/AsyncLoading.h"
#include "NilouType.h"
#include "Common/Paths.h"

namespace nilou {

class FUObjectHashTables
{
public:
    std::mutex CriticalSection;
    TMap<std::string, NObject*> ObjectMap;
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
    {
        return *Found;
    }

    return nullptr;
}

NObject* FindObject(NObject* Outer, const std::string& Name)
{
    if (Outer == nullptr)
    {
        return FindObject(Name);
    }
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

    if (!FPackageName::DoesPackageExist(Name)) return nullptr;

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
    Package = NewObject<NPackage>(nullptr, Name);
    Package->MarkPackageDirty();
    return Package;
}

NPackage* GObjTransientPkg = nullptr;
NPackage* GetTransientPackage()
{
    return GObjTransientPkg;
}

NObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params)
{    
    NObject* Object = Params.Class->CreateObject(Params.Name, Params.Outer, Params.Flags);
    return Object;
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
            if (Object == Package)
            {
                continue;
            }
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

FObjectInitializer& FObjectInitializer::Get()
{
    static thread_local FObjectInitializer Instance;
    return Instance;
}

NObject::NObject(const FObjectInitializer& Initializer)
    : ObjectFlags(Initializer.Flags)
    , ClassPrivate(Initializer.Class)
    , NamePrivate(Initializer.Name)
    , OuterPrivate(Initializer.Outer)
{
    FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
    std::lock_guard<std::mutex> Lock(ThreadHash.CriticalSection);
    ThreadHash.ObjectMap.Add(NamePrivate, this);
    if (OuterPrivate)
    {
        ThreadHash.ObjectOuterMap.FindOrAdd(OuterPrivate).Add(this);
    }
}

NObject::~NObject()
{

}

bool NObject::IsA(const NClass *Class) const
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
    ClearFlags(EObjectFlags::NeedLoad);
    nilou::Serialize(Ar["NamePrivate"], NamePrivate);   // This is necessary in case the object is the same with CDO.
    GetClass()->SerializeTaggedProperties(Ar, this);
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

void InitUObject()
{
    static bool bInitialized = false;
    if (bInitialized)
    {
        return;
    }
    bInitialized = true;
    FObjectInitializer& Initializer = FObjectInitializer::Get();

    Initializer.Class = nullptr;    // deferred 1
    Initializer.Name = "/Script/Engine";
    Initializer.Outer = nullptr;
    NPackage* NativeClassPackage = new NPackage;

    Initializer.Class = nullptr;    // deferred 2
    Initializer.Name = "Class";
    Initializer.Outer = NativeClassPackage;
    NClass* NClass_StaticClass = new NClass;
    NClass::Z_StaticClass = NClass_StaticClass;
    NClass_StaticClass->ClassPrivate = NClass_StaticClass;  // assgin deferred 2

    Initializer.Class = NClass::StaticClass();
    Initializer.Name = "Object";
    Initializer.Outer = NativeClassPackage;
    NClass* NObject_StaticClass = new NClass;
    NObject::Z_StaticClass = NObject_StaticClass;

    Initializer.Class = NClass::StaticClass();
    Initializer.Name = "Package";
    Initializer.Outer = NativeClassPackage;
    NClass* NPackage_StaticClass = new NClass;
    NPackage::Z_StaticClass = NPackage_StaticClass;
    NativeClassPackage->ClassPrivate = NPackage_StaticClass;    // assign deferred 1
    
    NClass_StaticClass->Size = sizeof(NClass);
    NClass_StaticClass->SuperStruct = NObject_StaticClass;
    NClass_StaticClass->ClassConstructor = [](void* Memory) { new (Memory) NClass(); };
    NClass_StaticClass->SetClassFlags(EClassFlags::Native | EClassFlags::Intrinsic);
    NClass_StaticClass->CreateDefaultObject();

    NObject_StaticClass->Size = sizeof(NObject);
    NObject_StaticClass->ClassConstructor = [](void* Memory) { new (Memory) NObject(); };
    NObject_StaticClass->SetClassFlags(EClassFlags::Native | EClassFlags::Intrinsic);
    NObject_StaticClass->CreateDefaultObject();

    NPackage_StaticClass->Size = sizeof(NPackage);
    NPackage_StaticClass->SuperStruct = NObject_StaticClass;
    NPackage_StaticClass->ClassConstructor = [](void* Memory) { new (Memory) NPackage(); };
    NPackage_StaticClass->SetClassFlags(EClassFlags::Native | EClassFlags::Intrinsic);
    NPackage_StaticClass->CreateDefaultObject();

    GObjTransientPkg = NewObject<NPackage>(nullptr, "/Engine/Transient");
}

NObject* NClass::CreateObject(const std::string& Name, NObject* Outer, EObjectFlags Flags) const
{
    void* Memory = malloc(Size);
    FObjectInitializer& Initializer = FObjectInitializer::Get();
    Initializer.Class = const_cast<NClass*>(this);
    Initializer.Name = Name;
    Initializer.Outer = Outer;
    Initializer.Flags = Flags;
    ClassConstructor(Memory);
    return static_cast<NObject*>(Memory);
}

TArray<FClassRegistryBase*> FClassRegistryBase::Registrations;
FClassRegistryBase::FClassRegistryBase(
    EMetaClass InMetaClass, 
    const std::string& InName, 
    NClass* InSuperClass, 
    int32 Size, 
    EClassFlags InClassFlags, 
    std::function<void(void*)> InClassConstructor)
{
    InitUObject();
    std::string Name = RemoveNamespace(InName);
    Name = RemovePrefix(InMetaClass, Name);
    FStaticConstructObjectParameters Params;
    Params.Class = NClass::StaticClass();
    Params.Outer = FindObject<NPackage>("/Script/Engine");
    Params.Name = Name;
    Class = Cast<NClass>(StaticConstructObject_Internal(Params));
    Class->Size = Size;
    Class->MetaClass = InMetaClass;
    Class->SuperStruct = InSuperClass;
    Class->ClassConstructor = InClassConstructor;
    Class->SetClassFlags(InClassFlags);
    Registrations.Add(this);
}

NClass* NObject::Z_StaticClass = nullptr;

}
