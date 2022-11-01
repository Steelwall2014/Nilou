#pragma once

#include <string>
#include "Platform.h"
namespace nilou {
    class FHashedName
    {
    public:
        FHashedName()
        { }
        FHashedName(const std::string &RawName)
        {
            Hash = std::hash<std::string>()(RawName);
#ifdef _DEBUG
            DebugString = RawName;
#endif
        }
    
        int64 Hash;
#ifdef _DEBUG
        std::string DebugString;
#endif

        bool operator==(const FHashedName &Other) const
        {
            return Hash == Other.Hash;
        }
    };
}

namespace std {
    template<>
    struct hash<nilou::FHashedName>
    {
        size_t operator()(const nilou::FHashedName &_Keyval) const noexcept {
            return _Keyval.Hash;
        }
    };
}

