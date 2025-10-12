#pragma once
#include <iostream>

#include "Common/CoreUObject/Class.h"

namespace nilou {

    class NCLASS NTestObjectC : public NObject
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int c;
    };
}