#include "Package.h"

namespace nilou {

void Serialize(FArchive& Ar, FObjectExport& Value)
{
    ::Serialize(Ar, (FObjectResource&)Value);
    ::Serialize(Ar, Value.ClassName);
    ::Serialize(Ar, Value.ObjectIndex);
}

void Serialize(FArchive& Ar, FObjectImport& Value)
{
    ::Serialize(Ar, (FObjectResource&)Value);
    ::Serialize(Ar, Value.PackageName);
}

void NPackage::Serialize(FArchive& Ar)
{
    NObject::Serialize(Ar);
    ::Serialize(Ar, ObjectExports);
    ::Serialize(Ar, ObjectImports);
}


}
