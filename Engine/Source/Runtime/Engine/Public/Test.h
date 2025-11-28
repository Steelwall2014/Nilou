#pragma once
#include "NObject/NilouType.h"
#include "Math/Transform.h"

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

        NPROPERTY()
        TArray<uint8> Binary;

        static FTestStructB Construct();
    };

    class NCLASS NTestObjectA : public NObject
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int Int;

        NPROPERTY()
        float Float;

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
        NTestObjectA* Self;

        NPROPERTY()
        NTestObjectB* Null = nullptr;

        NPROPERTY()
        NTestObjectC* Recursive;

        static NTestObjectA* Construct();
    };

    class NCLASS NTestObjectB : public NTestObjectA 
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int b;

        static NTestObjectB* Construct();
    };

    class NCLASS NTestObject : public NObject
    {
        GENERATED_BODY()
    public:
        NPROPERTY()
        TArray<NTestObject*> Children;
    };

    void TestSavePackage();
    void TestLoadPackage();

    void TestSaveDependencyPackage();
    void TestLoadDependencyPackage();
}