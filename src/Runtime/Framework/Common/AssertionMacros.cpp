#include "AssertionMacros.h"
#include <iostream>

namespace AssertionMacros {
    void zzzimpl_check(bool check_passed, const char *file, int line, const char *info)
    {
        if (!check_passed)
        {
            std::cout << "File: " << file << " Line: " << line << " Info: " << info << std::endl;
        }
    }
}
