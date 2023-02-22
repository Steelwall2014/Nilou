#include "../../src/Runtime/Rendering/UniformBuffer.h"
namespace nilou {
std::string UUniformBuffer::GetClassName() { return "UUniformBuffer"; }
EUClasses UUniformBuffer::GetClassEnum() { return EUClasses::MC_UUniformBuffer; }
const UClass *UUniformBuffer::GetClass() { return UUniformBuffer::StaticClass(); }
const UClass *UUniformBuffer::StaticClass()
{
	static UClass *StaticClass = new UClass("UUniformBuffer", EUClasses::MC_UUniformBuffer);
	return StaticClass;
}
}
