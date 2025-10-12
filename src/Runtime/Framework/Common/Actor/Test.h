#pragma once
#include <iostream>

#include "Common/CoreUObject/Class.h"

namespace nilou {

    class NTestObjectC;

    class NCLASS NTestObjectA : public NObject
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int a;

        NPROPERTY()
        NTestObjectC* C;
    };

    class NCLASS NTestObjectB : public NTestObjectA 
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int b;

        NPROPERTY()
        TArray<NTestObjectC*> CArray;

        NPROPERTY()
        TMap<int, NTestObjectC*> CMap;

        NPROPERTY()
        TSet<NTestObjectC*> CSet;
    };
}