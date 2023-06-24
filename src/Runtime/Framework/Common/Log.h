#pragma once

#include <iosfwd>
#include <string_view>
#include <iostream>
#include <format>
#include "AssertionMacros.h"

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

    enum class ELogLevel
    {
        LL_Info,
        LL_Warning,
        LL_Error,
        LL_Fatal
    };

    #define NILOU_LOG(Level, info, ...) \
        { \
            std::string str = std::format(info, __VA_ARGS__); \
            if constexpr (ELogLevel::LL_##Level == ELogLevel::LL_Info) \
            { \
                std::cout << "[INFO] " << str << std::endl; \
            } \
            else if constexpr (ELogLevel::LL_##Level == ELogLevel::LL_Warning) \
            { \
                std::cout << "[WARNING] " << str << std::endl; \
            } \
            else if constexpr (ELogLevel::LL_##Level == ELogLevel::LL_Error) \
            { \
                std::cout << "[ERROR] " << str << std::endl; \
            } \
            else if constexpr (ELogLevel::LL_##Level == ELogLevel::LL_Fatal) \
            { \
                std::cout << "[FATAL] " << str << std::endl; \
            } \
        } \


}