#pragma once

#define check(expr) \
    AssertionMacros::zzzimpl_check(expr, __FILE__, __LINE__, "CRITICAL WARNING: "#expr" check failed")

namespace AssertionMacros {
    void zzzimpl_check(bool check_passed, const char *file, int line, const char *info);
}