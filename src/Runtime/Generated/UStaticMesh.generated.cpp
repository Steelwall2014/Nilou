#include "D:/Nilou/src/Runtime/Framework/Common/StaticMeshResources.h"
namespace nilou {
std::string UStaticMesh::GetClassName() { return "UStaticMesh"; }
EUClasses UStaticMesh::GetClassEnum() { return EUClasses::MC_UStaticMesh; }
const UClass *UStaticMesh::GetClass() { return UStaticMesh::StaticClass(); }
const UClass *UStaticMesh::StaticClass()
{
	static UClass *StaticClass = new UClass("UStaticMesh", EUClasses::MC_UStaticMesh);
	return StaticClass;
}
std::unique_ptr<UObject> UStaticMesh::CreateDefaultObject()
{
    return std::make_unique<UStaticMesh>();
}
}
