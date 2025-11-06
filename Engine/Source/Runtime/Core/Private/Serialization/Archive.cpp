#include <fstream>
#include "Serialization/Archive.h"

namespace nilou {

    FArchive& FArchive::operator[](const std::string& Key)
    {
        if (!ObjectChildren.Contains(Key))
        {
            ObjectChildren[Key] = std::make_unique<FArchive>(Node[Key], bIsLoading);
        }
        return *ObjectChildren[Key];
    }

    FArchive& FArchive::operator[](size_t Index)
    {
        if (ArrayChildren.Num() <= Index)
        {
            ArrayChildren.SetNum(Index + 1);
            ArrayChildren[Index] = std::make_unique<FArchive>(Node[Index], bIsLoading);
        }
        return *ArrayChildren[Index];
    }
}