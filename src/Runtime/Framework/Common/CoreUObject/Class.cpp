#include "Class.h"
#include "Object.h"
#include "base64.h"

namespace nilou {

void FStructProperty::SerializeItem(FArchive& Ar, void* Value)
{
    Struct->SerializeFunction(Ar, Value);
}

void FObjectProperty::SerializeItem(FArchive& Ar, void* Value)
{
    Class->SerializeFunction(Ar, Value);
}

}
