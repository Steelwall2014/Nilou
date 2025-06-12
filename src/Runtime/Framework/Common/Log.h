#pragma once

#include <iosfwd>
#include <string_view>
#include <iostream>
#include <cassert>
#if __cplusplus >= 202002L
#include <format>
#else
#include <fmt/format.h>
#endif

#define PREPROCESSOR_JOIN(x, y) PREPROCESSOR_JOIN_INNER(x, y)
#define PREPROCESSOR_JOIN_INNER(x, y) x##y

namespace nilou {

    namespace encode {
        // 将 utf-16 字符串转换为特定 codepage 的多字节字符串存储到 std::string 中
        std::string wideToMulti(std::wstring_view sourceStr, unsigned int pagecode);

        std::string wideToUtf8(std::wstring_view sourceWStr);
        std::string wideToOme(std::wstring_view sourceWStr);
    }

    // ostream 特化, 针对 std::cout, std::cerr 等转换为本机 codepage 后以多字节编码输出
    // 不能表示的字节会被替换为 '?'
    std::ostream &operator<<(std::ostream &os, std::wstring_view str);
    // 需要显式转换到 std::wstring_view, 只是一层包装
    std::ostream &operator<<(std::ostream &os, const wchar_t *strPt);

    // 其他类型的输出流的模板, 输出存储在 std::string 中的 utf-8 字符串
    template <typename Ostream>
    Ostream &operator<<(Ostream &os, std::wstring_view str)
    {
        os << encode::wideToUtf8(str);
        return os;
    }

    template <typename Ostream>
    Ostream &operator<<(Ostream &os, const wchar_t *strPt)
    {
        return os << std::wstring_view(strPt);
    }

#if defined(_MSC_VER)
    #define NILOU_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
    #define NILOU_DEBUG_BREAK() __builtin_debugtrap()
#elif defined(__GNUC__)
    #define NILOU_DEBUG_BREAK() __builtin_trap()
#else
    #define NILOU_DEBUG_BREAK() assert(false)
#endif

#if __cplusplus >= 202002L
    #define NFormat(...) std::format(__VA_ARGS__)
#else
    #define NFormat(...) fmt::format(__VA_ARGS__)
#endif

    enum class ELogVerbosity
    {
        Fatal,
        Error,
        Warning,
        Display,
    };

    #define NILOU_LOG_EXPAND_IS_FATAL(Verbosity, ActiveBlock, InactiveBlock) NILOU_LOG_EXPAND_IS_FATAL_##Verbosity(ActiveBlock, InactiveBlock)

    #define NILOU_LOG_EXPAND_IS_FATAL_Fatal(ActiveBlock, InactiveBlock) ActiveBlock
    #define NILOU_LOG_EXPAND_IS_FATAL_Error(ActiveBlock, InactiveBlock) InactiveBlock
    #define NILOU_LOG_EXPAND_IS_FATAL_Warning(ActiveBlock, InactiveBlock) InactiveBlock
    #define NILOU_LOG_EXPAND_IS_FATAL_Display(ActiveBlock, InactiveBlock) InactiveBlock

    void Logf_Internal(ELogVerbosity Verbosity, const std::string& Message);

    #define NILOU_LOG(Verbosity, Format, ...) \
        { \
            auto NILOU_LOG_lambda = [](auto&&... args) { \
                Logf_Internal(nilou::ELogVerbosity::Verbosity, NFormat(#Verbosity": |{}:{}| " Format "\n", std::forward<decltype(args)>(args)...)); \
                NILOU_LOG_EXPAND_IS_FATAL(Verbosity, \
                    { \
                        NILOU_DEBUG_BREAK(); \
                        assert(false); \
                    },\
                    {} \
                ); \
            }; \
            NILOU_LOG_lambda(__FILE__, __LINE__, __VA_ARGS__); \
        }

#define Ncheck(expr) \
    if (!(expr)) \
    { \
        NILOU_LOG(Fatal, "Check failed {}", #expr); \
    }

#define Ncheckf(expr, format, ...) \
    if (!(expr)) \
    { \
        NILOU_LOG(Fatal, format, __VA_ARGS__); \
    }

#define NILOU_NOT_IMPLEMENTED { Ncheckf(false, "Not implemented"); }

}