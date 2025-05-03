#include "Transform.h"
#include <UDRefl/UDRefl.hpp>
using namespace Ubpa;
using namespace Ubpa::UDRefl;


template<>
struct TClassRegistry<nilou::FRotator>
{
    TClassRegistry(const std::string& InName)
    {
        Mngr.RegisterType<nilou::FRotator>();
		Mngr.AddField<&nilou::FRotator::Pitch>("Pitch");
		Mngr.AddField<&nilou::FRotator::Yaw>("Yaw");
		Mngr.AddField<&nilou::FRotator::Roll>("Roll");
    }

    static TClassRegistry<nilou::FRotator> Dummy;
};
TClassRegistry<nilou::FRotator> Dummy1 = TClassRegistry<nilou::FRotator>("nilou::FRotator");

template<>
struct TClassRegistry<nilou::FTransform>
{
    TClassRegistry(const std::string& InName)
    {
        Mngr.RegisterType<nilou::FTransform>();
		Mngr.AddField<&nilou::FTransform::Rotation>("Rotation");
		Mngr.AddField<&nilou::FTransform::Translation>("Translation");
		Mngr.AddField<&nilou::FTransform::Scale3D>("Scale3D");
    }

    static TClassRegistry<nilou::FTransform> Dummy;
};
TClassRegistry<nilou::FTransform> Dummy2 = TClassRegistry<nilou::FTransform>("nilou::FTransform");

namespace nilou {
    // 这里只能包在namespace里面, 如果这样写std::ostream &operator<<(std::ostream &out, const FTransform &obj)会报错

}

