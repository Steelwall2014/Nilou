#include "Test.h"
#include "Test2.h"
#include "NObject/Package.h"
#include "Misc/Paths.h"

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
    NPackage* PackageA = LoadPackage("/Test/Serialization/NTestObjectA/Package");
    NTestObjectA* ObjectA = FindObject<NTestObjectA>(PackageA, "TestObjectA");
}

NTestObject* CreateTestObject(std::string PackagePath)
{
	std::string ObjectName = FPaths::GetBaseFilename(PackagePath);
	NPackage* TestPackage = CreatePackage(PackagePath);
	NTestObject* TestObject = NewObject<NTestObject>(TestPackage, ObjectName);
	TestPackage->MarkPackageDirty();
	return TestObject;
}

void TestSaveDependencyPackage()
{
    /*

    N0
    |
    ↓
    N1<----
    |     |
    ↓     |
    N2--->N3
    |
    ↓
    N4<----
    |     |
    ↓     |
    N5--->N6

    */

	NTestObject* N0 = CreateTestObject("/Game/TestPackage0");
	NTestObject* N1 = CreateTestObject("/Game/TestPackage1");
	NTestObject* N2 = CreateTestObject("/Game/TestPackage2");
	NTestObject* N3 = CreateTestObject("/Game/TestPackage3");
	NTestObject* N4 = CreateTestObject("/Game/TestPackage4");
	NTestObject* N5 = CreateTestObject("/Game/TestPackage5");
	NTestObject* N6 = CreateTestObject("/Game/TestPackage6");

	N0->Children.Add(N1);
	N1->Children.Add(N2);
	N2->Children.Add(N3);
	N3->Children.Add(N1);
	N2->Children.Add(N4);
	N4->Children.Add(N5);
	N5->Children.Add(N6);
	N6->Children.Add(N4);

	NPackage::SavePackage(N0->GetPackage());
	NPackage::SavePackage(N1->GetPackage());
	NPackage::SavePackage(N2->GetPackage());
	NPackage::SavePackage(N3->GetPackage());
	NPackage::SavePackage(N4->GetPackage());
	NPackage::SavePackage(N5->GetPackage());
	NPackage::SavePackage(N6->GetPackage());
}

void TestLoadDependencyPackage()
{
    LoadPackage("/Game/TestPackage1");
}

}