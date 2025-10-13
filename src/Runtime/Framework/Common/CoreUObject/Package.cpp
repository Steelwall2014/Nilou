#include "Package.h"
#include "Class.h"
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
}

void Serialize(FArchive& Ar, FObjectExport& Value)
{
    Serialize(Ar, (FObjectResource&)Value);
    Serialize(Ar["ClassName"], Value.ClassName);
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
    TArray<FObjectImport> ObjectImports;
    TArray<FObjectExport> ObjectExports;
    TArray<NObject*> InnnerObjects = GetObjectsWithPackage(Package, true);
    TSet<NObject*> ImportObjects;
    TSet<NObject*> ExportObjects;
    for (NObject* Obj : InnnerObjects)
    {
        TSet<NObject*> References;
        Obj->GetObjectReferences(References);
        for (NObject* Ref : References)
        {
            NPackage* RefPackage = Ref->GetPackage();
            if (RefPackage != Package)
            {
                while (Ref)
                {
                    ImportObjects.Add(Ref);
                    Ref = Ref->GetOuter();
                }
            }
        }
        ExportObjects.Add(Obj);
    }
    TMap<NObject*, FPackageIndex> ObjectIndexMap;
    for (NObject* Obj : ImportObjects)
    {
        FObjectImport Import;
        Import.ObjectName = Obj->GetName();
        Import.PackageName = Obj->GetPackage()->GetName();
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
    HarvestPackage(Package);
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
    TArray<NObject*> InnnerObjects = GetObjectsWithPackage(Package, true);
    nlohmann::json File;
    nlohmann::json& ObjectsJson = File["Objects"];
    FArchive Ar(ObjectsJson, false);
    for (int32 ObjectIndex = 0; ObjectIndex < InnnerObjects.Num(); ++ObjectIndex)
    {
        NObject* Obj = InnnerObjects[ObjectIndex];
        Obj->Serialize(Ar[ObjectIndex]);
    }
    std::ofstream out(FileName);
    out << File.dump(4);
}

void HarvestPackage(NPackage* Package)
{
    TSet<NObject*> Imports;
    TArray<NObject*> Exports = GetObjectsWithPackage(Package, true);
    for (int32 i = 0; i < Exports.Num(); ++i)
    {
        NClass* Class = Exports[i]->GetClass();
        Imports.Add(Class);
        NObject* CDO = Class->GetDefaultObject();
        Imports.Add(CDO);
        struct FExport
        {
            void* Data;
            NClass* Class;
        };
        TArray<FExport> Stack;
        Stack.Add(FExport{Exports[i], Class});
        auto VisitContainerItem = [&](void* ContainerItem, FProperty* ItemProperty)
        {
            if (FStructProperty* Inner = CastField<FStructProperty>(ItemProperty))
            {
                Stack.Add(FExport{ContainerItem, Inner->Struct});
            }
            else if (FObjectProperty* Inner = CastField<FObjectProperty>(ItemProperty))
            {
                NObject* ItemObject = *reinterpret_cast<NObject**>(ContainerItem);
                if (ItemObject)
                {
                    if (ItemObject->GetPackage() != Package)
                    {
                        Imports.Add(ItemObject);
                    }
                    Stack.Add(FExport{ItemObject, ItemObject->GetClass()});
                }
            }
        };
        while (!Stack.IsEmpty())
        {
            FExport Export = Stack.Pop();
            TArray<FProperty*> Properties = Export.Class->GetProperties(true);
            for (FProperty* Property : Properties)
            {
                void* Field = Property->ContainerPtrToValuePtrInternal(Export.Data);
                if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
                {
                    Stack.Add(FExport{Field, StructProperty->Struct});
                }
                else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
                {
                    NObject* Object = *reinterpret_cast<NObject**>(Field);
                    if (Object)
                    {
                        if (Object->GetPackage() != Package)
                        {
                            Imports.Add(Object);
                        }
                        Stack.Add(FExport{Object, Object->GetClass()});
                    }
                }
                else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
                {
                    for (int32 Index = 0; Index < ArrayProperty->GetNum(Field); ++Index)
                    {
                        void* Item = ArrayProperty->GetItem(Field, Index);
                        VisitContainerItem(Item, ArrayProperty->Inner);
                    }
                }
                else if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
                {
                    TArray<void*> Pairs = MapProperty->GetPairs(Field);
                    for (void* Pair : Pairs)
                    {
                        void* Key = MapProperty->PairGetKey(Pair);
                        void* Value = MapProperty->PairGetValue(Pair);
                        VisitContainerItem(Key, MapProperty->KeyProp);
                        VisitContainerItem(Value, MapProperty->ValueProp);
                    }
                }
                else if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
                {
                    TArray<void*> Items = SetProperty->GetItems(Field);
                    for (void* Item : Items)
                    {
                        VisitContainerItem(Item, SetProperty->Inner);
                    }
                }
            }
        }
    }
}


}
