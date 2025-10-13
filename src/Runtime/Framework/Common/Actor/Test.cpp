#include "Test.h"
#include "Test2.h"
#include "Common/CoreUObject/Package.h"

namespace nilou {

FTestStructA FTestStructA::Construct()
{
    FTestStructA Struct;
    for (int i = 0; i < 4; i++)
    {
        NPackage* Package = CreatePackage("/Test/Serailization/FTestStructA/Array/Package_" + std::to_string(i));
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_" + std::to_string(i));
        Struct.BArray.Add(Object);
    }
    for (int i = 0; i < 5; i++)
    {
        NPackage* Package = CreatePackage("/Test/Serailization/FTestStructA/Map/Package_" + std::to_string(i));
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_" + std::to_string(i));
        Struct.BMap.Add(std::to_string(i), Object);
    }
    for (int i = 0; i < 6; i++)
    {
        NPackage* Package = CreatePackage("/Test/Serailization/FTestStructA/Set/Package_" + std::to_string(i));
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_" + std::to_string(i));
        Struct.BSet.Add(Object);
    }
    return Struct;
}

FTestStructB FTestStructB::Construct()
{
    FTestStructB Struct;
    Struct.A = FTestStructA::Construct();
    return Struct;
}

NTestObjectA* NTestObjectA::Construct()
{
    NPackage* Package = CreatePackage("/Test/Serialization/NTestObjectA/Package");
    NTestObjectA* Object = NewObject<NTestObjectA>(Package, "TestObjectA");
    
    // 设置基本属性
    Object->Int = 42;
    Object->String = "Hello Serialization";
    Object->Vector = FVector(1.0f, 2.0f, 3.0f);
    Object->Quat = FQuat(0.0f, 0.0f, 0.0f, 1.0f);
    Object->Transform = FTransform(FVector(10.0f, 20.0f, 30.0f), FQuat(0.0f, 0.0f, 0.0f, 1.0f), FVector(1.0f, 1.0f, 1.0f));
    
    // 创建结构体数组
    for (int i = 0; i < 3; i++)
    {
        Object->StructArray.Add(FTestStructB::Construct());
    }
    
    // 创建C对象引用（同一Package内）
    Object->C = NewObject<NTestObjectC>(Package, "TestObjectC_SamePackage");
    Object->C->c = 100;
    
    return Object;
}

NTestObjectB* NTestObjectB::Construct()
{
    NPackage* Package = CreatePackage("/Test/Serialization/NTestObjectB/Package");
    NTestObjectB* Object = NewObject<NTestObjectB>(Package, "TestObjectB");
    
    // 继承自NTestObjectA的属性
    Object->Int = 84;
    Object->String = "Hello NTestObjectB";
    Object->Vector = FVector(4.0f, 5.0f, 6.0f);
    Object->Quat = FQuat(0.1f, 0.2f, 0.3f, 0.9f);
    Object->Transform = FTransform(FVector(40.0f, 50.0f, 60.0f), FQuat(0.1f, 0.2f, 0.3f, 0.9f), FVector(2.0f, 2.0f, 2.0f));
    
    // 创建结构体数组
    for (int i = 0; i < 2; i++)
    {
        Object->StructArray.Add(FTestStructB::Construct());
    }
    
    // 创建C对象引用（同一Package内）
    Object->C = NewObject<NTestObjectC>(Package, "TestObjectC_SamePackageB");
    Object->C->c = 200;
    
    // NTestObjectB特有的属性
    Object->b = 168;
    
    // 创建C对象数组（同一Package内）
    for (int i = 0; i < 3; i++)
    {
        NPackage* InnerPackage = CreatePackage("/Test/Serialization/NTestObjectB/Array/Package_" + std::to_string(i));
        NTestObjectC* CObject = NewObject<NTestObjectC>(InnerPackage, "ObjectC_" + std::to_string(i));
        CObject->c = 300 + i;
        Object->CArray.Add(CObject);
    }
    
    // 创建C对象Map（同一Package内）
    for (int i = 0; i < 4; i++)
    {
        NPackage* InnerPackage = CreatePackage("/Test/Serialization/NTestObjectB/Map/Package_" + std::to_string(i));
        NTestObjectC* CObject = NewObject<NTestObjectC>(InnerPackage, "ObjectC_Map_" + std::to_string(i));
        CObject->c = 400 + i;
        Object->CMap.Add(i, CObject);
    }
    
    // 创建C对象Set（同一Package内）
    for (int i = 0; i < 5; i++)
    {
        NPackage* InnerPackage = CreatePackage("/Test/Serialization/NTestObjectB/Set/Package_" + std::to_string(i));
        NTestObjectC* CObject = NewObject<NTestObjectC>(InnerPackage, "ObjectC_Set_" + std::to_string(i));
        CObject->c = 500 + i;
        Object->CSet.Add(CObject);
    }
    
    return Object;
}

// 序列化测试用例
void TestSerialization()
{
    NILOU_LOG(Display, "=== Starting Serialization Test ===");
    
    // 1. 测试同一Package内的Object引用
    NILOU_LOG(Display, "\n1. Testing Object References within Same Package");
    NTestObjectA* TestObjectA = NTestObjectA::Construct();
    NILOU_LOG(Display, "Created NTestObjectA: {}", TestObjectA->GetPathName());
    NILOU_LOG(Display, "Int: {}", TestObjectA->Int);
    NILOU_LOG(Display, "String: {}", TestObjectA->String);
    NILOU_LOG(Display, "Vector: ({}, {}, {})", TestObjectA->Vector.x, TestObjectA->Vector.y, TestObjectA->Vector.z);
    NILOU_LOG(Display, "C: {}, c值: {}", TestObjectA->C->GetPathName(), TestObjectA->C->c);
    
    // 保存Package
    NPackage* PackageA = TestObjectA->GetPackage();
    NILOU_LOG(Display, "Saving Package: {}", PackageA->GetName());
    NPackage::SavePackage(PackageA);
    
    // 2. 测试不同Package间的Object引用
    NILOU_LOG(Display, "\n2. Testing Object References across Different Packages");
    NTestObjectB* TestObjectB = NTestObjectB::Construct();
    NILOU_LOG(Display, "Created NTestObjectB: {}", TestObjectB->GetPathName());
    NILOU_LOG(Display, "Int: {}", TestObjectB->Int);
    NILOU_LOG(Display, "b: {}", TestObjectB->b);
    NILOU_LOG(Display, "C: {}, c值: {}", TestObjectB->C->GetPathName(), TestObjectB->C->c);
    
    // 创建跨Package引用
    NPackage* CrossPackage = CreatePackage("/Test/Serialization/CrossPackage/Package");
    NTestObjectA* CrossObjectA = NewObject<NTestObjectA>(CrossPackage, "CrossObjectA");
    CrossObjectA->Int = 999;
    CrossObjectA->String = "Cross Package Reference";
    
    // 引用不同Package中的对象
    CrossObjectA->C = TestObjectB->C;  // 引用TestObjectB Package中的C对象
    NILOU_LOG(Display, "Created Cross-Package Reference Object: {}", CrossObjectA->GetPathName());
    NILOU_LOG(Display, "Cross-Package Referenced C Object: {}, c value: {}", CrossObjectA->C->GetPathName(), CrossObjectA->C->c);
    
    // 保存所有Package
    NPackage* PackageB = TestObjectB->GetPackage();
    NILOU_LOG(Display, "Saving PackageB: {}", PackageB->GetName());
    NPackage::SavePackage(PackageB);
    
    NILOU_LOG(Display, "Saving CrossPackage: {}", CrossPackage->GetName());
    NPackage::SavePackage(CrossPackage);
    
    // 3. 测试加载Package
    NILOU_LOG(Display, "\n3. Testing Package Loading");
    
    // 加载PackageA
    NPackage* LoadedPackageA = LoadPackage("/Test/Serialization/NTestObjectA/Package");
    if (LoadedPackageA)
    {
        NILOU_LOG(Display, "Successfully loaded PackageA: {}", LoadedPackageA->GetName());
        NTestObjectA* LoadedObjectA = FindObject<NTestObjectA>(LoadedPackageA, "TestObjectA");
        if (LoadedObjectA)
        {
            NILOU_LOG(Display, "Loaded NTestObjectA: {}", LoadedObjectA->GetPathName());
            NILOU_LOG(Display, "Int: {}", LoadedObjectA->Int);
            NILOU_LOG(Display, "String: {}", LoadedObjectA->String);
            NILOU_LOG(Display, "Vector: ({}, {}, {})", LoadedObjectA->Vector.x, LoadedObjectA->Vector.y, LoadedObjectA->Vector.z);
            if (LoadedObjectA->C)
            {
                NILOU_LOG(Display, "C Object Reference: {}, c value: {}", LoadedObjectA->C->GetPathName(), LoadedObjectA->C->c);
            }
        }
    }
    
    // 加载PackageB
    NPackage* LoadedPackageB = LoadPackage("/Test/Serialization/NTestObjectB/Package");
    if (LoadedPackageB)
    {
        NILOU_LOG(Display, "Successfully loaded PackageB: {}", LoadedPackageB->GetName());
        NTestObjectB* LoadedObjectB = FindObject<NTestObjectB>(LoadedPackageB, "TestObjectB");
        if (LoadedObjectB)
        {
            NILOU_LOG(Display, "Loaded NTestObjectB: {}", LoadedObjectB->GetPathName());
            NILOU_LOG(Display, "Int: {}", LoadedObjectB->Int);
            NILOU_LOG(Display, "b: {}", LoadedObjectB->b);
            if (LoadedObjectB->C)
            {
                NILOU_LOG(Display, "C Object Reference: {}, c value: {}", LoadedObjectB->C->GetPathName(), LoadedObjectB->C->c);
            }
            NILOU_LOG(Display, "CArray size: {}", LoadedObjectB->CArray.Num());
            NILOU_LOG(Display, "CMap size: {}", LoadedObjectB->CMap.Num());
            NILOU_LOG(Display, "CSet size: {}", LoadedObjectB->CSet.Num());
        }
    }
    
    // 加载CrossPackage
    NPackage* LoadedCrossPackage = LoadPackage("/Test/Serialization/CrossPackage/Package");
    if (LoadedCrossPackage)
    {
        NILOU_LOG(Display, "Successfully loaded CrossPackage: {}", LoadedCrossPackage->GetName());
        NTestObjectA* LoadedCrossObjectA = FindObject<NTestObjectA>(LoadedCrossPackage, "CrossObjectA");
        if (LoadedCrossObjectA)
        {
            NILOU_LOG(Display, "Loaded CrossObjectA: {}", LoadedCrossObjectA->GetPathName());
            NILOU_LOG(Display, "Int: {}", LoadedCrossObjectA->Int);
            NILOU_LOG(Display, "String: {}", LoadedCrossObjectA->String);
            if (LoadedCrossObjectA->C)
            {
                NILOU_LOG(Display, "Cross-Package Referenced C Object: {}, c value: {}", LoadedCrossObjectA->C->GetPathName(), LoadedCrossObjectA->C->c);
            }
        }
    }
    
    NILOU_LOG(Display, "\n=== Serialization Test Completed ===");
}

}