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

void Serialize(FArchive& Ar, FObjectExport& Value)
{
    Serialize(Ar, (FObjectResource&)Value);
    Serialize(Ar, Value.ClassName);
    Serialize(Ar, Value.ObjectIndex);
}

void Serialize(FArchive& Ar, FObjectImport& Value)
{
    Serialize(Ar, (FObjectResource&)Value);
    Serialize(Ar, Value.PackageName);
}

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
    std::string MetaFileName = FPackagePath::LongPackageNameToMetaFileName(Package->GetName());
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
    for (NObject* Obj : InnnerObjects)
    {
        {
            nlohmann::json& ObjJson = ObjectsJson.emplace_back();
            FArchive Ar(File);
            Obj->Serialize(Ar);
            std::ofstream out(FileName);
            out << File.dump(4);
        }
    }
}


}
