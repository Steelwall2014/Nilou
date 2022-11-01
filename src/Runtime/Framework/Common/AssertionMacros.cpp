#include "AssertionMacros.h"
#include <iostream>

namespace AssertionMacros {
    void zzzimpl_check(bool check_passed, const char *info)
    {
        if (!check_passed)
        {
            std::cout << info << std::endl;
        }
    }
}
