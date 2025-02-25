#pragma once
// #if __cplusplus >= 202002L
// #include <format>
// #define NILOU_LOG_FORMAT(t, ...) std::format(t, __VA_ARGS__)
// #else
// #include <fmt/format.h>
// #define NILOU_LOG_FORMAT(t, ...) fmt::format(t, __VA_ARGS__)
// #endif

// #define Ncheck(expr) \
//     AssertionMacros::zzzimpl_check((expr), (__FILE__), (__LINE__), "CRITICAL WARNING: "#expr" check failed")

// #define Ncheckf(expr, format, ...) \
//     AssertionMacros::zzzimpl_checkf((expr), __FILE__, __LINE__, format, __VA_ARGS__)

// namespace AssertionMacros {
//     void zzzimpl_check(bool check_passed, const char *file, int line, const char *info);
//     template<typename... T>
//     void zzzimpl_checkf(bool check_passed, const char *file, int line, const char *format, T... args)
//     {
//         std::string info = NILOU_LOG_FORMAT(format, args...);
//         zzzimpl_check(check_passed, file, line, info.c_str());
//     }
// }