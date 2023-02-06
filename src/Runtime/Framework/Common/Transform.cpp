#include "Transform.h"


namespace nilou {

    template<> const FTransform3f FTransform3f::Identity = FTransform3f();

    template<>const  FTransform FTransform::Identity = FTransform();
    
    template<> const FRotator3f FRotator3f::ZeroRotator = FRotator3f();

    template<> const FRotator FRotator::ZeroRotator = FRotator();



}

namespace nilou {
    // 这里只能包在namespace里面, 如果这样写std::ostream &operator<<(std::ostream &out, const FTransform &obj)会报错

}

