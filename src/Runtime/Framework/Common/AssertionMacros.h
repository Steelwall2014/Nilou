#pragma once
#include <fmt/format.h>

#define Ncheck(expr) \
    AssertionMacros::zzzimpl_check((expr), (__FILE__), (__LINE__), "CRITICAL WARNING: "#expr" check failed")

#define Ncheckf(expr, format, ...) \
    AssertionMacros::zzzimpl_checkf((expr), __FILE__, __LINE__, format, __VA_ARGS__)

namespace AssertionMacros {
    void zzzimpl_check(bool check_passed, const char *file, int line, const char *info);
    template<typename... T>
    void zzzimpl_checkf(bool check_passed, const char *file, int line, const char *format, T... args)
    {
        std::string info = fmt::format(format, args...);
        zzzimpl_check(check_passed, file, line, info.c_str());
    }
}