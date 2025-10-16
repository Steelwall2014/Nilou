#include "Test.h"
#include "Test2.h"
#include "Common/CoreUObject/Package.h"

namespace nilou {

FTestStructA FTestStructA::Construct()
{
    NPackage* Package = FindPackage("/Test/Serialization/NTestObjectA/Package");
    FTestStructA Struct;
    for (int i = 0; i < 1; i++)
    {
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_Array_" + std::to_string(i));
        Struct.BArray.Add(Object);
    }
    Package = CreatePackage("/Test/Serialization/NTestObjectB/Package");
    for (int i = 0; i < 2; i++)
    {
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_Map_" + std::to_string(i));
        Struct.BMap.Add(std::to_string(i), Object);
    }
    for (int i = 0; i < 3; i++)
    {
        NTestObjectB* Object = NewObject<NTestObjectB>(Package, "ObjectB_Set_" + std::to_string(i));
        Struct.BSet.Add(Object);
    }
    return Struct;
}

FTestStructB FTestStructB::Construct()
{
    FTestStructB Struct;
    Struct.A = FTestStructA::Construct();
    Struct.Binary = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    return Struct;
}

NTestObjectA* NTestObjectA::Construct()
{
    NPackage* PackageA = CreatePackage("/Test/Serialization/NTestObjectA/Package");
    NTestObjectA* ObjectA = NewObject<NTestObjectA>(PackageA, "TestObjectA");
    
    // 设置基本属性
    ObjectA->Int = 42;
    ObjectA->Float = 1.0f;
    ObjectA->String = "Hello Serialization";
    ObjectA->Vector = FVector(1.0f, 2.0f, 3.0f);
    ObjectA->Quat = FQuat(0.0f, 0.0f, 0.0f, 1.0f);
    ObjectA->Transform = FTransform(FVector(10.0f, 20.0f, 30.0f), FQuat(0.0f, 0.0f, 0.0f, 1.0f), FVector(1.0f, 1.0f, 1.0f));
    ObjectA->Self = ObjectA;
    
    // 创建结构体数组
    for (int i = 0; i < 1; i++)
    {
        ObjectA->StructArray.Add(FTestStructB::Construct());
    }
    
    NPackage* PackageC = CreatePackage("/Test/Serialization/NTestObjectC/Package");
    ObjectA->Recursive = NewObject<NTestObjectC>(PackageC, "TestObjectC");
    ObjectA->Recursive->c = 100;
    ObjectA->Recursive->Recursive = ObjectA;
    
    return ObjectA;
}

// 序列化测试用例
void TestSavePackage()
{
    NILOU_LOG(Display, "=== Starting Serialization Test ===");
    NTestObjectA::Construct();
    NPackage::SavePackage(FindPackage("/Test/Serialization/NTestObjectA/Package"));
    NPackage::SavePackage(FindPackage("/Test/Serialization/NTestObjectB/Package"));
    NPackage::SavePackage(FindPackage("/Test/Serialization/NTestObjectC/Package"));
}

void TestLoadPackage()
{
    NILOU_LOG(Display, "=== Starting Deserialization Test ===");
}

}