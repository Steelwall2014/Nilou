#pragma once

#define check(expr) AssertionMacros::zzzimpl_check(expr, "CRITICAL WARNING: "#expr" check failed")

namespace AssertionMacros {
    void zzzimpl_check(bool check_passed, const char *info);
}