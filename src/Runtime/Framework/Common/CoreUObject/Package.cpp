#include "Package.h"
#include "Class.h"
#include "NilouType.h"
#include "Common/Containers/Map.h"
#include "Common/Containers/Set.h"
#include "Common/Path.h"

namespace nilou {

void Serialize(FArchive& Ar, FPackageIndex& Value)
{
    Serialize(Ar, Value.Index);
}

void Serialize(FArchive& Ar, FObjectResource& Value)
{
    Serialize(Ar["ObjectName"], Value.ObjectName);
    Serialize(Ar["OuterIndex"], Value.OuterIndex);
    Serialize(Ar["ClassName"], Value.ClassName);
}

void Serialize(FArchive& Ar, FObjectExport& Value)
{
    Serialize(Ar, (FObjectResource&)Value);
    Serialize(Ar["ClassIndex"], Value.ClassIndex);
    Serialize(Ar["ObjectIndex"], Value.ObjectIndex);
}

void Serialize(FArchive& Ar, FObjectImport& Value)
{
    Serialize(Ar, (FObjectResource&)Value);
    Serialize(Ar["PackageName"], Value.PackageName);
}

NClass* NPackage::Z_StaticClass = nullptr;

void NPackage::Serialize(FArchive& Ar)
{
    NObject::Serialize(Ar);
}

std::pair<TArray<FObjectImport>, TArray<FObjectExport>> BuildLinker(NPackage* Package)
{
    TSet<NObject*> ImportObjects, ExportObjects;
    HarvestPackage(Package, ImportObjects, ExportObjects);
    TArray<FObjectImport> ObjectImports;
    TArray<FObjectExport> ObjectExports;
    TMap<NObject*, FPackageIndex> ObjectIndexMap;
    for (NObject* Obj : ImportObjects)
    {
        FObjectImport Import;
        Import.ObjectName = Obj->GetName();
        Import.PackageName = Obj->GetPackage()->GetName();
        Import.ClassName = Obj->GetClass()->GetName();
        Import.XObject = Obj;
        ObjectIndexMap.Add(Obj, FPackageIndex::FromImport(ObjectImports.Num()));
        ObjectImports.Add(Import);
    }
    for (NObject* Obj : ExportObjects)
    {
        FObjectExport Export;
        Export.ObjectName = Obj->GetName();
        Export.ClassName = Obj->GetClass()->GetName();
        Export.Object = Obj;
        Export.ObjectIndex = FPackageIndex::FromExport(ObjectExports.Num());
        Export.ClassIndex = ObjectIndexMap[Obj->GetClass()];
        ObjectIndexMap.Add(Obj, Export.ObjectIndex);
        ObjectExports.Add(Export);
    }
    for (FObjectImport& Imports : ObjectImports)
    {
        NObject* Obj = Imports.XObject;
        NObject* Outer = Obj->GetOuter();
        if (Outer)
        {
            Imports.OuterIndex = ObjectIndexMap[Outer];
        }
    }
    for (FObjectExport& Export : ObjectExports)
    {
        NObject* Obj = Export.Object;
        Export.ClassIndex = ObjectIndexMap[Obj->GetClass()];
        NObject* Outer = Obj->GetOuter();
        if (Outer)
        {
            Export.OuterIndex = ObjectIndexMap[Outer];
        }
    }
    return {ObjectImports, ObjectExports};
}

void NPackage::SavePackage(NPackage* Package)
{
    std::string MetaFileName = FPackagePath::LongPackageNameToMetaFileName(Package->GetName());
    std::string DirectoryName = std::filesystem::path(MetaFileName).parent_path().string();
    std::filesystem::create_directories(DirectoryName);
    auto [ObjectImports, ObjectExports] = BuildLinker(Package);
    {
        nlohmann::json MetaFile;
        {
            FArchive Ar(MetaFile["ObjectImports"], false);
            nilou::Serialize(Ar, ObjectImports);
        }
        {
            FArchive Ar(MetaFile["ObjectExports"], false);
            nilou::Serialize(Ar, ObjectExports);
        }
        std::ofstream out(MetaFileName);
        out << MetaFile.dump(4);
    }
    std::string FileName = FPackagePath::LongPackageNameToFileName(Package->GetName());
    nlohmann::json File;
    nlohmann::json& ObjectsJson = File["Objects"];
    FArchive Ar(ObjectsJson, false);
    for (int32 ObjectIndex = 0; ObjectIndex < ObjectExports.Num(); ++ObjectIndex)
    {
        NObject* Obj = ObjectExports[ObjectIndex].Object;
        Obj->Serialize(Ar[ObjectIndex]);
    }
    std::ofstream out(FileName);
    out << File.dump(4);
}

struct FPackageHarvester
{
    TSet<NObject*>& Imports;
    TSet<NObject*>& Exports;
    TSet<void*> Visited;
    NPackage* Package;
    TArray<std::pair<void*, FProperty*>> Stack;
    FPackageHarvester(NPackage* InPackage, TSet<NObject*>& InImports, TSet<NObject*>& InExports) 
        : Package(InPackage)
        , Imports(InImports)
        , Exports(InExports)
    {
        TArray<NObject*> ExportsArray = GetObjectsWithPackage(Package, true);
        for (NObject* Export : ExportsArray)
        {
            AddReference(Export);
        }
    }
    void AddReference(NObject* Ref)
    {
        if (!Ref) return;
        if (Ref->GetPackage() != Package)
        {
            while (Ref)
            {
                Imports.Add(Ref);
                Ref = Ref->GetOuter();
            }
        }
        else 
        {
            Exports.Add(Ref);
            NClass* Class = Ref->GetClass();
            AddReference(Class);
            TArray<FProperty*> Properties = Class->GetProperties(true);
            for (FProperty* Property : Properties)
            {
                void* Field = Property->ContainerPtrToValuePtrInternal(Ref);
                Push(Field, Property);
            }
        }
    }
    bool IsEmpty() const
    {
        return Stack.IsEmpty();
    }
    void Push(void* Field, FProperty* Property)
    {
        if (Property->IsA<FStructProperty>() ||
            Property->IsA<FObjectProperty>() ||
            Property->IsA<FArrayProperty>() ||
            Property->IsA<FMapProperty>() ||
            Property->IsA<FSetProperty>())
        {
            if (!Visited.Contains(Field))
            {
                Visited.Add(Field);
                Stack.Add({Field, Property});
            }
        }
    }
    std::pair<void*, FProperty*> Pop()
    {
        std::pair<void*, FProperty*> Reference = Stack.Pop();
        return Reference;
    }
};

void HarvestPackage(NPackage* Package, TSet<NObject*>& Imports, TSet<NObject*>& Exports)
{
    FPackageHarvester Harvester(Package, Imports, Exports);
    while (!Harvester.IsEmpty())
    {
        auto [Field, Property] = Harvester.Pop();
        if (auto ObjectProperty = CastField<FObjectProperty>(Property))
        {
            NObject* Ref = *(NObject**)Field;
            Harvester.AddReference(Ref);
        }
        else if (auto StructProperty = CastField<FStructProperty>(Property))
        {
            TArray<FProperty*> InnerProperties = StructProperty->Struct->GetProperties(true);
            for (FProperty* InnerProperty : InnerProperties)
            {
                void* InnerField = InnerProperty->ContainerPtrToValuePtrInternal(Field);
                Harvester.Push(InnerField, InnerProperty);
            }
        }
        else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
        {
            for (int32 Index = 0; Index < ArrayProperty->GetNum(Field); ++Index)
            {
                void* Item = ArrayProperty->GetItem(Field, Index);
                Harvester.Push(Item, ArrayProperty->Inner);
            }
        }
        else if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
        {
            TArray<void*> Pairs = MapProperty->GetPairs(Field);
            for (void* Pair : Pairs)
            {
                void* Key = MapProperty->PairGetKey(Pair);
                void* Value = MapProperty->PairGetValue(Pair);
                Harvester.Push(Key, MapProperty->KeyProp);
                Harvester.Push(Value, MapProperty->ValueProp);
            }
        }
        else if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
        {
            TArray<void*> Items = SetProperty->GetItems(Field);
            for (void* Item : Items)
            {
                Harvester.Push(Item, ArrayProperty->Inner);
            }
        }
    }
}


}
