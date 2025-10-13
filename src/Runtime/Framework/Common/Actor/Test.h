#pragma once
#include <iostream>

#include "Common/CoreUObject/Class.h"
#include "Common/Math/Transform.h"

namespace nilou {

    class NTestObjectB;
    class NTestObjectC;

    struct NSTRUCT FTestStructA
    {
        GENERATED_BODY()

        NPROPERTY()
        TArray<NTestObjectB*> BArray;

        NPROPERTY()
        TMap<std::string, NTestObjectB*> BMap;

        NPROPERTY()
        TSet<NTestObjectB*> BSet;

        static FTestStructA Construct();
    };

    struct NSTRUCT FTestStructB
    {
        GENERATED_BODY()

        NPROPERTY()
        FTestStructA A;

        static FTestStructB Construct();
    };

    class NCLASS NTestObjectA : public NObject
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int Int;

        NPROPERTY()
        std::string String;

        NPROPERTY()
        FVector Vector;

        NPROPERTY()
        FQuat Quat;

        NPROPERTY()
        FTransform Transform;

        NPROPERTY()
        TArray<FTestStructB> StructArray;

        NPROPERTY()
        NTestObjectC* C;

        static NTestObjectA* Construct();
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

        static NTestObjectB* Construct();
    };

    // 单元测试入口函数
    void TestSerialization();
}